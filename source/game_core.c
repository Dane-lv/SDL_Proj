#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include <math.h>     
#include <string.h>
#include "../include/game_core.h"
#include "../include/constants.h"
#include "../include/player.h"
#include "../include/camera.h"
#include "../include/maze.h"
#include "../include/projectile.h"
#include "../include/network.h"

#define UPDATE_RATE 10  // Send position updates every 10 frames

// Helper function to set window title with connection info
static void setWindowTitle(GameContext *ctx, const char *title) {
    if (ctx->window) {
        SDL_SetWindowTitle(ctx->window, title);
    }
}

bool gameInit(GameContext *ctx) {
    // Players array initialization
    for (int i = 0; i < MAX_PLAYERS; i++) {
        ctx->players[i] = NULL;
    }

    // Create local player
    ctx->localPlayer = createPlayer(ctx->renderer);
    if (!ctx->localPlayer) {
        printf("Error creating player: %s\n", SDL_GetError());
        return false;
    }
    
    // Store local player in player array (will be updated when ID is assigned)
    ctx->players[0] = ctx->localPlayer;

    // Create maze
    ctx->maze = createMaze(ctx->renderer, NULL, NULL);  // Removed texture references
    if (!ctx->maze) {
        printf("Maze Creation Error: %s\n", SDL_GetError());
        return false;
    }
    initiateMap(ctx->maze);
    generateMazeLayout(ctx->maze);
    
    // Create camera
    ctx->camera = createCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!ctx->camera) {
        printf("Error: Failed to create camera\n");
        return false;
    }

    // Initialize projectiles
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        ctx->projectiles[i] = createProjectile(ctx->renderer);
        if (!ctx->projectiles[i]) {
            printf("Error: Failed to create projectile %d\n", i);
            return false;
        }
    }
    
    // Set window title based on connection type
    if (ctx->isNetworked) {
        if (ctx->isHost) {
            setWindowTitle(ctx, "Maze Mayhem - HOST");
            
            // For host, player ID is always 0
            setPlayerPosition(ctx->localPlayer, 400, 300);
        } else {
            setWindowTitle(ctx, "Maze Mayhem - CLIENT");
        }
    } else {
        setWindowTitle(ctx, "Maze Mayhem - OFFLINE");
        setPlayerPosition(ctx->localPlayer, 400, 300);
    }

    ctx->isRunning = true;
    ctx->frameCounter = 0;
    
    return true;
}

void gameCoreRunFrame(GameContext *ctx) {
    static Uint32 lastTime = 0;
    static bool initialPosSet = false;

    // Initialize lastTime on first call
    if (lastTime == 0) {
        lastTime = SDL_GetTicks();
        initialPosSet = ctx->isHost; // Already set for host
    }

    // Handle time and delta time
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    
    // Handle events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            ctx->isRunning = false;
        } else {
            handleInput(ctx, &event);
        }
    }
    
    // Update game state
    updateGame(ctx, deltaTime);
    updatePlayerRotation(ctx);
    
    // Network update
    if (ctx->isNetworked) {
        if (ctx->isHost) {
            hostTick(&ctx->netMgr, ctx);
        } else {
            clientTick(&ctx->netMgr, ctx);
            
            // If we're a client and we just got assigned a player ID, set initial position
            if (!initialPosSet && ctx->netMgr.localPlayerId != 0xFF) {
                // Set different starting positions based on player ID
                float startX = 300.0f + (ctx->netMgr.localPlayerId * 100);
                float startY = 200.0f + (ctx->netMgr.localPlayerId * 80);
                setPlayerPosition(ctx->localPlayer, startX, startY);
                initialPosSet = true;
                
                // Update our local player index in the players array
                if (ctx->players[0] == ctx->localPlayer) {
                    ctx->players[0] = NULL;
                }
                ctx->players[ctx->netMgr.localPlayerId] = ctx->localPlayer;
                
                // Immediately send our position to other players
                SDL_Rect pos = getPlayerPosition(ctx->localPlayer);
                float angle = getPlayerAngle(ctx->localPlayer);
                sendPlayerPosition(&ctx->netMgr, (float)pos.x, (float)pos.y, angle);
            }
        }
        
        // Send position updates periodically to reduce network traffic
        ctx->frameCounter++;
        if (ctx->frameCounter >= UPDATE_RATE) {
            ctx->frameCounter = 0;
            
            // Only send if we have a valid player ID
            if (ctx->netMgr.localPlayerId != 0xFF) {
                SDL_Rect pos = getPlayerPosition(ctx->localPlayer);
                float angle = getPlayerAngle(ctx->localPlayer);
                sendPlayerPosition(&ctx->netMgr, (float)pos.x, (float)pos.y, angle);
            }
        }
    }
    
    // Render frame
    renderGame(ctx);
}

void handleInput(GameContext *ctx, SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                ctx->isRunning = false;
                break;
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                movePlayerUp(ctx->localPlayer);
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                movePlayerDown(ctx->localPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                movePlayerLeft(ctx->localPlayer);
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                movePlayerRight(ctx->localPlayer);
                break;
            case SDL_SCANCODE_SPACE:
                spawnProjectile(ctx->projectiles, ctx->localPlayer);
                
                // Send shoot event over network
                if (ctx->isNetworked && ctx->netMgr.localPlayerId != 0xFF) {
                    SDL_Rect playerPos = getPlayerPosition(ctx->localPlayer);
                    float playerX = (float)(playerPos.x + playerPos.w/2);
                    float playerY = (float)(playerPos.y + playerPos.h/2);
                    float angle = getPlayerAngle(ctx->localPlayer);
                    sendPlayerShoot(&ctx->netMgr, playerX, playerY, angle);
                }
                break;
            default:
                break;
        }
    } else if (event->type == SDL_KEYUP) {
        switch (event->key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_DOWN:
                stopMovementVY(ctx->localPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_RIGHT:
                stopMovementVX(ctx->localPlayer);
                break;
            default:
                break;
        }
    }
}

void updateGame(GameContext *ctx, float deltaTime) {

    updatePlayer(ctx->localPlayer, deltaTime);
    
    SDL_Rect playerRect = getPlayerRect(ctx->localPlayer);
    bool collision = checkCollision(ctx->maze, playerRect);
    if (collision) {
        revertToPreviousPosition(ctx->localPlayer);
    }
    
    updateProjectileWithWallCollision(ctx->projectiles, ctx->maze, deltaTime);
    
    updateCamera(ctx->camera, ctx->localPlayer);
}

void updatePlayerRotation(GameContext *ctx) {
  
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
 
    SDL_Rect playerPos = getPlayerPosition(ctx->localPlayer);
    
    SDL_Rect screenPlayerPos = getWorldCoordinatesFromCamera(ctx->camera, playerPos);
    
   
    float playerCenterX = screenPlayerPos.x + screenPlayerPos.w / 2.0f;
    float playerCenterY = screenPlayerPos.y + screenPlayerPos.h / 2.0f;
    
    // Calculate angle between player center and mouse
    float deltaX = mouseX - playerCenterX;
    float deltaY = mouseY - playerCenterY;
    float radians = atan2f(deltaY, deltaX);
    float angle = radians * 180.0f / 3.14;
    
    // Update player angle
    setPlayerAngle(ctx->localPlayer, angle);
}

void renderGame(GameContext *ctx) {
  
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);
    
    drawMap(ctx->maze, ctx->camera, ctx->localPlayer);
    
   
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (ctx->players[i] != NULL) {
            drawPlayer(ctx->players[i], ctx->camera);
        }
    }
    
   
    drawProjectile(ctx->projectiles, ctx->camera);

   
    SDL_RenderPresent(ctx->renderer);
}

void gameCoreShutdown(GameContext *ctx) {
  
    destroyProjectile(ctx->projectiles);

    if (ctx->camera) {
        destroyCamera(ctx->camera);
        ctx->camera = NULL;
    }

    // Clean up all players (except local player which is handled separately)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (ctx->players[i] && ctx->players[i] != ctx->localPlayer) {
            destroyPlayer(ctx->players[i]);
            ctx->players[i] = NULL;
        }
    }

    if (ctx->localPlayer) {
        destroyPlayer(ctx->localPlayer);
        ctx->localPlayer = NULL;
    }

    if (ctx->maze) {
        destroyMaze(ctx->maze);
        ctx->maze = NULL;
    }
}

void gameOnNetworkMessage(GameContext *ctx, Uint8 type, Uint8 playerId, const void *data, int size) {
    if (playerId >= MAX_PLAYERS) return;
    
    switch (type) {
        case MSG_JOIN: {
            if (ctx->netMgr.localPlayerId == playerId) {
                if (ctx->players[0] == ctx->localPlayer) {
                    ctx->players[0] = NULL;
                }
                ctx->players[playerId] = ctx->localPlayer;
            }
            break;
        }
        case MSG_POS: {
            if (size < 3.0 * sizeof(float)) return;

            float* posData = (float*)data;
            float x = posData[0];
            float y = posData[1];
            float angle = posData[2];
            
            // skapa icke lokal spelare 
            if (ctx->players[playerId] == NULL && playerId != ctx->netMgr.localPlayerId) {
                ctx->players[playerId] = createPlayer(ctx->renderer);
                if (!ctx->players[playerId]) return;
                
                // positions  of players
                setPlayerPosition(ctx->players[playerId], x, y);
                setPlayerAngle(ctx->players[playerId], angle);
                
                printf("Created remote player with ID %d\n", playerId);
            }
            
            // Skip updating if it's our local player
            if (playerId != ctx->netMgr.localPlayerId) {
                // updatera spelaren
                setPlayerPosition(ctx->players[playerId], x, y);
                setPlayerAngle(ctx->players[playerId], angle);
            }
            break;
        }
        case MSG_SHOOT: {
            if (size < 3.0 * sizeof(float)) return;

            if (playerId == ctx->netMgr.localPlayerId) return;
            
            // Ensure remote player exists
            if (ctx->players[playerId] == NULL) return;
            
            // Spawn projectile for remote player
            spawnProjectile(ctx->projectiles, ctx->players[playerId]);
            break;
        }
        case MSG_LEAVE: {
            
                if (playerId != ctx->netMgr.localPlayerId && ctx->players[playerId]) {
                destroyPlayer(ctx->players[playerId]);
                ctx->players[playerId] = NULL;
            }
            break;
        }
    }
} 