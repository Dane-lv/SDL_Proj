#include <stdio.h>
#include <SDL.h>
#include <SDL_net.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <string.h>

#include "../include/constants.h"
#include "../include/game_core.h"
#include "../include/network.h"
#include "../include/menu.h"
#include "../include/audio_manager.h"
#include "../include/lobby.h"

#define DEFAULT_PORT 7777
#define DEFAULT_IP   "127.0.0.1"

/* --------------------------------------------------------- */
static void cleanupNetwork(NetMgr *nm)
{
    if (nm->client) SDLNet_TCP_Close(nm->client);
    if (nm->server) {
        for (int i = 0; i < nm->peerCount; ++i)
            if (nm->peers[i]) SDLNet_TCP_Close(nm->peers[i]);
        SDLNet_TCP_Close(nm->server);
    }
    if (nm->set) SDLNet_FreeSocketSet(nm->set);
    memset(nm, 0, sizeof *nm);
}

/* --------------------------------------------------------- */
int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO) != 0) {
        SDL_Log("SDL_Init: %s", SDL_GetError()); return 1;
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init: %s", TTF_GetError()); SDL_Quit(); return 1;
    }
    if (!netInit()) {
        SDL_Log("SDL_net: %s", SDLNet_GetError());
        TTF_Quit(); SDL_Quit(); return 1;
    }

    GameContext ctx = { .isRunning = true };
    ctx.window = SDL_CreateWindow("Maze Mayhem",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!ctx.window) { SDL_Log("Window: %s", SDL_GetError()); return 1; }

    ctx.renderer = SDL_CreateRenderer(ctx.window, -1,
                    SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE);
    if (!ctx.renderer){ SDL_Log("Renderer: %s", SDL_GetError()); return 1; }

    ctx.audioManager = createAudioManager();
    if (ctx.audioManager) playBackgroundMusic(ctx.audioManager);

    /* ----------------------------------------------------- */
    while (ctx.isRunning)            /* yttre huvud-loop  */
    {
        /* ----- MENY ------------------------------------- */
        Menu *menu = menuCreate(ctx.renderer, ctx.window, &ctx);
        while (ctx.isRunning) {
            SDL_Event ev; while (SDL_PollEvent(&ev))
                if (!menuHandleEvent(menu, &ev)) break;
            menuRender(menu);
            if (menuGetChoice(menu) != MENU_CHOICE_NONE) break;
            SDL_Delay(16);
        }
        MenuChoice mc = menuGetChoice(menu);
        menuDestroy(menu);

        if (!ctx.isRunning || mc == MENU_CHOICE_QUIT || mc == MENU_CHOICE_NONE)
            break;                          /* stäng programmet */

        /* ----- NÄTVERK ---------------------------------- */
        bool isHost = (mc == MENU_CHOICE_HOST);
        char ipBuf[64] = DEFAULT_IP;
        if (mc == MENU_CHOICE_JOIN)
            strncpy(ipBuf, menuGetJoinIP(menu), 63), ipBuf[63]='\0';

        /* ev. init-om SDL_net om man har gått BACK tidigare */
        if (!ctx.netMgr.set && !ctx.netMgr.server && !ctx.netMgr.client)
            if (!netInit()) { SDL_Log("SDL_net re-init fail"); break; }

        bool ok = isHost
            ? hostStart (&ctx.netMgr, DEFAULT_PORT)
            : clientConnect(&ctx.netMgr, ipBuf, DEFAULT_PORT);
        if (!ok) { SDL_Log("Network error:%s", SDLNet_GetError()); continue; }

        ctx.isHost = isHost; ctx.isNetworked = true;
        ctx.netMgr.userData = &ctx;
        ctx.lobbyReceivedStart = false;

        /* ----- LOBBY ------------------------------------ */
        Lobby *lob = lobbyCreate(ctx.renderer, ctx.window, &ctx, isHost);
        bool startGame = false, goBack = false;

        while (ctx.isRunning && !startGame && !goBack) {
            SDL_Event ev; while (SDL_PollEvent(&ev))
                lobbyHandleEvent(lob, &ev);

            if (isHost) hostTick  (&ctx.netMgr, &ctx);
            else        clientTick(&ctx.netMgr, &ctx);

            if (isHost && lobbyIsReady(lob)) {
                sendStartGame(&ctx.netMgr);
                startGame = true;
            }
            if (!isHost && ctx.lobbyReceivedStart)
                startGame = true;

            if (lobbyBackPressed(lob))
                goBack = true;

            lobbyRender(lob);
            SDL_Delay(16);
        }
        lobbyDestroy(lob);

        if (goBack) {              /* stäng nät & åter till menyn */
            cleanupNetwork(&ctx.netMgr);
            netShutdown();         /* stäng SDL_net helt          */
            continue;              /* hoppa till yttre while (=meny) */
        }

        if (!startGame) break;     /* program avslutas t.ex. vid quit */

        /* ----- STARTA SPELET ------------------------------------ */
        if (!gameInit(&ctx)) { SDL_Log("gameInit fail"); break; }
        while (ctx.isRunning) gameCoreRunFrame(&ctx);
        gameCoreShutdown(&ctx);
        break;                     /* efter spel → avsluta hela appen */
    }

    /* --------------------------------------------------------- */
    cleanupNetwork(&ctx.netMgr);
    netShutdown();
    if (ctx.audioManager) destroyAudioManager(ctx.audioManager);
    if (ctx.renderer) SDL_DestroyRenderer(ctx.renderer);
    if (ctx.window)   SDL_DestroyWindow(ctx.window);
    TTF_Quit(); SDL_Quit();
    return 0;
}