
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "../include/constants.h"
#include "../include/game_core.h"
#include "../include/player.h"
#include "../include/camera.h"
#include "../include/maze.h"
#include "../include/projectile.h"
#include "../include/network.h"
#include "../include/audio_manager.h"

/* hur ofta lokala positioner pushas ut på nätet */
#define UPDATE_RATE 10        /* var 10:e bildruta */

static void setWindowTitle     (GameContext *, const char *);
static void initDeathScreen    (GameContext *);
static void renderDeathScreen  (GameContext *);
static void enableSpectateMode (GameContext *);
static void checkPlayerProjectileCollisions(GameContext *);

/* ==========================================================
 *                 INITIALISERING
 * ========================================================== */
static void setWindowTitle(GameContext *g, const char *title)
{
    if (g->window) SDL_SetWindowTitle(g->window, title);
}

/* ---------------------------------------------------------- */
bool gameInit(GameContext *g)
{
    /* ---------------- spelare / projektilarrayer ---------- */
    for (int i = 0; i < MAX_PLAYERS; ++i) g->players[i] = NULL;

    g->localPlayer = createPlayer(g->renderer);
    if (!g->localPlayer) { SDL_Log("createPlayer fail"); return false; }

    if (g->isHost)
        playerSetTextureById(g->localPlayer, g->renderer, 0);

    g->players[0] = g->localPlayer;

    /* ---------------- Maze -------------------------------- */
    g->maze = createMaze(g->renderer, NULL, NULL);
    if (!g->maze) return false;
    initiateMap(g->maze);
    generateMazeLayout(g->maze);

    /* ---------------- Kamera ------------------------------ */
    g->camera = createCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!g->camera) return false;

    /* ---------------- Projektiler ------------------------- */
    for (int i = 0; i < MAX_PROJECTILES; ++i) {
        g->projectiles[i] = createProjectile(g->renderer);
        if (!g->projectiles[i]) return false;
    }

    /* ---------------- Ljud (om ej redan init) ------------- */
    if (!g->audioManager) {
        g->audioManager = createAudioManager();
        if (g->audioManager)
            playBackgroundMusic(g->audioManager);
    }

    /* ---------------- Död-/spectate-UI -------------------- */
    g->isSpectating   = false;
    g->showDeathScreen = false;

    SDL_Surface *s =
        TTF_RenderText_Blended(TTF_OpenFont("resources/font.ttf", 64),
                               "YOU DIED",
                               (SDL_Color){255, 0, 0, 255});
    g->fontTexture = s ? SDL_CreateTextureFromSurface(g->renderer, s) : NULL;
    if (s) SDL_FreeSurface(s);

    initDeathScreen(g);

    /* ---------------- Första spawn-placering -------------- */
    if (g->isNetworked) {
        if (g->isHost) {
            setWindowTitle(g, "Maze Mayhem - HOST");
            setPlayerPosition(g->localPlayer, 50.0f, 50.0f);            /* TL */
        } else {
            setWindowTitle(g, "Maze Mayhem - CLIENT");
            /* klient får exakt pos först när host svarat med ID */
        }
    } else {
        setWindowTitle(g, "Maze Mayhem - OFFLINE");
        setPlayerPosition(g->localPlayer, 400, 300);
    }

    g->isRunning    = true;
    g->frameCounter = 0;
    return true;
}

/* ==========================================================
 *                 HUVUD-LOOP (en bildruta)
 * ========================================================== */

 void startPosForId(int id, float *x, float *y)
{
    const float M = 50.f;                    /* marginal mot yttervägg */

    switch (id)
    {
        case 0:  *x = M; *y = M;                                   break; /* TL */
        case 1:  *x = WORLD_WIDTH  - PLAYERWIDTH  - M;
                 *y = M;                                           break; /* TR */
        case 2:  *x = M;
                 *y = WORLD_HEIGHT - PLAYERHEIGHT - M;             break; /* BL */
        case 3:  *x = WORLD_WIDTH  - PLAYERWIDTH  - M;
                 *y = WORLD_HEIGHT - PLAYERHEIGHT - M;             break; /* BR */

        /* Spelare 5 (ID 4) – garanterad golvyta strax under mitten */
        case 4:  *x = WORLD_WIDTH  * 0.5f - PLAYERWIDTH  * 0.5f + TILE_SIZE+2;
                 *y = WORLD_HEIGHT * 0.5f - PLAYERHEIGHT * 0.5f
                       + 2 * TILE_SIZE;                            break;

        default: /* fallback om MAX_PLAYERS höjs någon gång */
                 *x = 100.f + id * 40.f;
                 *y = 100.f;                                       break;
    }
}
void gameCoreRunFrame(GameContext *g)
{
    static Uint32 lastTime = 0;
    static bool   initialClientPosSet = false;

    if (lastTime == 0) {
        lastTime = SDL_GetTicks();
        initialClientPosSet = g->isHost;  /* host fick spawn direkt */
    }

    Uint32 now = SDL_GetTicks();
    float  dt  = (now - lastTime) / 1000.0f;
    lastTime   = now;

    /* ----------- indata ---------------------------------- */
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) g->isRunning = false;
        else handleInput(g, &ev);
    }

    /* ----------- logik ----------------------------------- */
    updateGame(g, dt);
    updatePlayerRotation(g);

    /* ----------- nätverk --------------------------------- */
    if (g->isNetworked) {
        if (g->isHost)
            hostTick(&g->netMgr, g);
        else {
            clientTick(&g->netMgr, g);
            if (!initialClientPosSet && g->netMgr.localPlayerId != 0xFF) 
            {

                float sx, sy;                       /* --- NYTT --- */
                startPosForId(g->netMgr.localPlayerId, &sx, &sy);
                setPlayerPosition(g->localPlayer, sx, sy);

                initialClientPosSet = true;

                playerSetTextureById(g->localPlayer, g->renderer,
                                     g->netMgr.localPlayerId);

                if (g->players[0] == g->localPlayer) g->players[0] = NULL;
                g->players[g->netMgr.localPlayerId] = g->localPlayer;

                SDL_Rect p = getPlayerPosition(g->localPlayer);
                sendPlayerPosition(&g->netMgr, (float)p.x, (float)p.y,
                                   getPlayerAngle(g->localPlayer));
            
            }
        }

        /* skicka egen pos mer sällan ----------------------- */
        if (++g->frameCounter >= UPDATE_RATE) {
            g->frameCounter = 0;
            if (g->netMgr.localPlayerId != 0xFF) {
                SDL_Rect p = getPlayerPosition(g->localPlayer);
                sendPlayerPosition(&g->netMgr, (float)p.x, (float)p.y,
                                   getPlayerAngle(g->localPlayer));
            }
        }
    }

    /* ----------- rendering ------------------------------- */
    renderGame(g);
}

/* ==========================================================
 *                 INDATBEHANDLING
 * ========================================================== */
void handleInput(GameContext *g, SDL_Event *e)
{
    /* Om död-skärm aktiv → bara mus-klick */
    if (!isPlayerAlive(g->localPlayer) && g->showDeathScreen) {
        if (e->type == SDL_MOUSEBUTTONDOWN) {
            int mx, my; SDL_GetMouseState(&mx, &my);
            SDL_Rect r = g->spectateButtonRect;
            if (mx >= r.x && mx <= r.x + r.w &&
                my >= r.y && my <= r.y + r.h)
                enableSpectateMode(g);
        }
        return;
    }

    /* Vanliga spel-tangenter -------------------------------- */
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.scancode) {
        case SDL_SCANCODE_ESCAPE: g->isRunning = false; break;
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP:    movePlayerUp   (g->localPlayer); break;
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:  movePlayerDown (g->localPlayer); break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT:  movePlayerLeft (g->localPlayer); break;
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT: movePlayerRight(g->localPlayer); break;

        case SDL_SCANCODE_SPACE:
            if (isPlayerAlive(g->localPlayer)) {
                int pid = spawnProjectile(g->projectiles, g->localPlayer);
                if (pid >= 0 && g->isNetworked) {
                    SDL_Rect p = getPlayerPosition(g->localPlayer);
                    float ang = getPlayerAngle(g->localPlayer);
                    float x = p.x + p.w / 2.0f;
                    float y = p.y + p.h / 2.0f;
                    float rad = ang * M_PI / 180.0f;
                    x += cosf(rad) * 5.0f;
                    y += sinf(rad) * 5.0f;
                    sendPlayerShoot(&g->netMgr, x, y, ang, pid);

                    sendPlayerPosition(&g->netMgr, (float)p.x, (float)p.y, ang);
                }
            }
            break;
        default: break;
        }
    }
    else if (e->type == SDL_KEYUP) {
        switch (e->key.keysym.scancode) {
        case SDL_SCANCODE_W:
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_S:
        case SDL_SCANCODE_DOWN:  stopMovementVY(g->localPlayer); break;
        case SDL_SCANCODE_A:
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_D:
        case SDL_SCANCODE_RIGHT: stopMovementVX(g->localPlayer); break;
        default: break;
        }
    }
}

/* ==========================================================
 *                 SPEL-UPPDATERING
 * ========================================================== */
void updateGame(GameContext *g, float dt)
{
    updatePlayer(g->localPlayer, dt);

    /* Håll spelaren inom världen */
    SDL_Rect r = getPlayerPosition(g->localPlayer);
    if (r.x < 0 || r.x + r.w > WORLD_WIDTH ||
        r.y < 0 || r.y + r.h > WORLD_HEIGHT)
        revertToPreviousPosition(g->localPlayer);

    /* Kollision mot väggar */
    if (checkCollision(g->maze, r))
        revertToPreviousPosition(g->localPlayer);

    /* Projektiler ----------------------------------------- */
    updateProjectileWithWallCollision(g->projectiles, g->maze, dt);

    /* Koll. projektil ↔ spelare */
    checkPlayerProjectileCollisions(g);

    /* Kamera ---------------------------------------------- */
    if (g->isSpectating) {
        float cx = (TILE_WIDTH  * TILE_SIZE) / 2.0f;
        float cy = (TILE_HEIGHT * TILE_SIZE) / 2.0f;
        setCameraPosition(g->camera, cx, cy);
    } else {
        updateCamera(g->camera, g->localPlayer);
    }
}

/* ---------------------------------------------------------- */
void updatePlayerRotation(GameContext *g)
{
    int mx, my; SDL_GetMouseState(&mx, &my);
    SDL_Rect pr = getPlayerPosition(g->localPlayer);
    SDL_Rect scr = getWorldCoordinatesFromCamera(g->camera, pr);

    float pcx = scr.x + scr.w / 2.0f;
    float pcy = scr.y + scr.h / 2.0f;

    float dx = mx - pcx;
    float dy = my - pcy;
    float ang = atan2f(dy, dx) * 180.0f / (float)M_PI;
    setPlayerAngle(g->localPlayer, ang);
}

/* ==========================================================
 *                 RENDERING
 * ========================================================== */
void renderGame(GameContext *g)
{
    SDL_SetRenderDrawColor(g->renderer, 10, 10, 10, 255);
    SDL_RenderClear(g->renderer);

    drawMap(g->maze, g->camera, g->localPlayer, g->isSpectating);

    for (int i = 0; i < MAX_PLAYERS; ++i) {
        Player *p = g->players[i];
        if (!p) continue;

        /* Host-/lokal spelare eller spectate → rita fullt */
        if (p == g->localPlayer || g->isSpectating) {
            drawPlayer(p, g->camera);
            continue;
        }

        /* ljus-faktor (samma formel som i drawMap) */
        SDL_Rect lr = getPlayerPosition(g->localPlayer);
        SDL_Rect pr = getPlayerPosition(p);

        float lcx = lr.x + lr.w * 0.5f;
        float lcy = lr.y + lr.h * 0.5f;
        float pcx = pr.x + pr.w * 0.5f;
        float pcy = pr.y + pr.h * 0.5f;
        float dx  = pcx - lcx;
        float dy  = pcy - lcy;
        float dist = sqrtf(dx*dx + dy*dy);

        float t = 1.f - dist / PLAYER_VISUAL_DIST;
        if (t < 0.f) t = 0.f;
        float b = FOG_MIN_BRIGHTNESS + t * (1.f - FOG_MIN_BRIGHTNESS);

        /* modulerar texturen               -------- NYTT -------- */
        SDL_Texture *tex = getPlayerTexture(p);
        if (tex) {
            Uint8 mod = (Uint8)(255.f * b);
            SDL_SetTextureColorMod(tex, mod, mod, mod);
            SDL_SetTextureAlphaMod(tex, mod);
        }

        drawPlayer(p, g->camera);

        /* återställ mod                -------- NYTT -------- */
        if (tex) {
            SDL_SetTextureColorMod(tex, 255, 255, 255);
            SDL_SetTextureAlphaMod(tex, 255);
        }
    }
    drawProjectile(g->projectiles, g->camera);

    if (!isPlayerAlive(g->localPlayer) && g->showDeathScreen)
        renderDeathScreen(g);

    SDL_RenderPresent(g->renderer);
}

/* ==========================================================
 *                 NEDSTÄNGNING
 * ========================================================== */
void gameCoreShutdown(GameContext *g)
{
    if (g->isNetworked) netShutdown();

    destroyProjectile(g->projectiles);
    destroyPlayer(g->localPlayer);
    for (int i = 0; i < MAX_PLAYERS; ++i)
        if (g->players[i] && g->players[i] != g->localPlayer)
            destroyPlayer(g->players[i]);

    destroyMaze(g->maze);
    destroyCamera(g->camera);

    if (g->fontTexture) SDL_DestroyTexture(g->fontTexture);

    if (g->audioManager) destroyAudioManager(g->audioManager);
}

/* ==========================================================
 *                 NÄTVERKS-CALLBACK
 * ========================================================== */
void gameOnNetworkMessage(GameContext *g, Uint8 type, Uint8 id,
                          const void *data, int size)
{
    if (id >= MAX_PLAYERS) return;

    switch (type) {
/* ---------------------------------------------------------- */
    case MSG_JOIN:
        if (g->netMgr.localPlayerId == id) {
            if (g->players[0] == g->localPlayer) g->players[0] = NULL;
            g->players[id] = g->localPlayer;
        }
        break;

/* ---------------------------------------------------------- */
    case MSG_POS:
        if (size < 3 * (int)sizeof(float)) return;

        if (id != g->netMgr.localPlayerId) {
            if (!g->players[id]) {
                g->players[id] = createPlayer(g->renderer);
                if (!g->players[id]) return;
                playerSetTextureById(g->players[id], g->renderer, id);
            }
            const float *p = (const float *)data;
            setPlayerPosition(g->players[id], p[0], p[1]);
            setPlayerAngle   (g->players[id], p[2]);
        }
        break;

/* ---------------------------------------------------------- */
    case MSG_SHOOT:
        if (size < 3 * (int)sizeof(float) + (int)sizeof(int)) return;
        if (id == g->netMgr.localPlayerId) return;
        if (!g->players[id]) return;

        {
            const float *p = (const float *)data;
            int pid = *((const int *)(p + 3));
            if (pid < 0 || pid >= MAX_PROJECTILES) return;

            float x = p[0], y = p[1], ang = p[2];
            float rad = ang * M_PI / 180.0f;

            setProjectileActive(g->projectiles[pid], true);
            setProjectileOwner (g->projectiles[pid], g->players[id]);
            setProjectilePosition(g->projectiles[pid], x, y);
            setProjectileVelocity(g->projectiles[pid],
                                  cosf(rad) * PROJSPEED,
                                  sinf(rad) * PROJSPEED);
            setProjectileDuration(g->projectiles[pid], 3.0f);
        }
        break;

/* ---------------------------------------------------------- */
    case MSG_LEAVE:
        if (id != g->netMgr.localPlayerId && g->players[id]) {
            destroyPlayer(g->players[id]);
            g->players[id] = NULL;
        }
        break;

/* ---------------------------------------------------------- */
    case MSG_DEATH:
        if (size < (int)sizeof(Uint8)) return;
        if (!g->players[id]) return;

        if (g->players[id] == g->localPlayer) {
            g->showDeathScreen = true;
            if (g->audioManager) playDeathSound(g->audioManager);
        }
        killPlayer(g->players[id]);
        break;

/* ---------------------------------------------------------- */
    case MSG_START:
        g->lobbyReceivedStart = true;   /* används av lobby-koden */
        break;
    }
}

/* ==========================================================
 *             KOLLISION: PROJ ↔ SPELARE
 * ========================================================== */
static void checkPlayerProjectileCollisions(GameContext *g)
{
    for (int i = 0; i < MAX_PROJECTILES; ++i)
        if (isProjectileActive(g->projectiles[i])) {

            /* mot lokal spelare ---------------------------- */
            if (isPlayerAlive(g->localPlayer) &&
                checkProjectilePlayerCollision(g->projectiles[i],
                                               g->localPlayer)) {
                killPlayer(g->localPlayer);
                deactivateProjectile(g->projectiles[i]);
                g->showDeathScreen = true;
                if (g->audioManager) playDeathSound(g->audioManager);

                if (g->isNetworked) {
                    Player *killer = getProjectileOwner(g->projectiles[i]);
                    Uint8 killerId = 0xFF;
                    for (int j = 0; j < MAX_PLAYERS; ++j)
                        if (g->players[j] == killer) { killerId = j; break; }
                    sendPlayerDeath(&g->netMgr, killerId);
                }
            }

            /* mot fjärr-spelare ---------------------------- */
            if (g->isNetworked)
                for (int j = 0; j < MAX_PLAYERS; ++j)
                    if (g->players[j] && g->players[j] != g->localPlayer &&
                        isPlayerAlive(g->players[j]) &&
                        checkProjectilePlayerCollision(g->projectiles[i],
                                                       g->players[j])) {

                        killPlayer(g->players[j]);
                        deactivateProjectile(g->projectiles[i]);
                    }
        }
}

/* ==========================================================
 *             DÖD-SKÄRM & SPECTATE
 * ========================================================== */
static void initDeathScreen(GameContext *g)
{
    int bw = 200, bh = 50;
    g->spectateButtonRect =
        (SDL_Rect){WINDOW_WIDTH / 2 - bw / 2,
                   WINDOW_HEIGHT / 2 + 50, bw, bh};
}

static void renderDeathScreen(GameContext *g)
{
    SDL_Renderer *r = g->renderer;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_RenderFillRect(r, &(SDL_Rect){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT});

    if (g->fontTexture) {
        SDL_Rect tr = {WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50,
                       300, 80};
        SDL_SetTextureColorMod(g->fontTexture, 255, 0, 0);
        SDL_RenderCopy(r, g->fontTexture, NULL, &tr);
    }

    SDL_SetRenderDrawColor(r, 100, 100, 100, 255);
    SDL_RenderFillRect(r, &g->spectateButtonRect);
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_RenderDrawRect(r, &g->spectateButtonRect);

    TTF_Font *f = TTF_OpenFont("resources/font.ttf", 28);
    if (f) {
        SDL_Surface *s = TTF_RenderText_Blended(
            f, "SPECTATE", (SDL_Color){255, 255, 255, 255});
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
            SDL_Rect dst = {g->spectateButtonRect.x +
                                (g->spectateButtonRect.w - s->w) / 2,
                            g->spectateButtonRect.y +
                                (g->spectateButtonRect.h - s->h) / 2,
                            s->w, s->h};
            SDL_RenderCopy(r, t, NULL, &dst);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
        }
        TTF_CloseFont(f);
    }
}

static void enableSpectateMode(GameContext *g)
{
    g->isSpectating   = true;
    g->showDeathScreen = false;
    setCameraSpectateMode(g->camera, true);

    float cx = (TILE_WIDTH  * TILE_SIZE) / 2.0f;
    float cy = (TILE_HEIGHT * TILE_SIZE) / 2.0f;
    setCameraPosition(g->camera, cx, cy);
}