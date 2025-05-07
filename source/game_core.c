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
static void setWindowTitle(GameContext *game, const char *title) {
    if (game->window) {
        SDL_SetWindowTitle(game->window, title);
    }
}

bool gameInit(GameContext *game) {
    // Players array initialization
    for (int i = 0; i < MAX_PLAYERS; i++) {
        game->players[i] = NULL;
    }

    // Create local player
    game->localPlayer = createPlayer(game->renderer);
    if (!game->localPlayer) {
        printf("Error creating player: %s\n", SDL_GetError());
        return false;
    }
    
    // Store local player in player array (will be updated when ID is assigned)
    game->players[0] = game->localPlayer;

    // Create maze
    game->maze = createMaze(game->renderer, NULL, NULL);  // Removed texture references
    if (!game->maze) {
        printf("Maze Creation Error: %s\n", SDL_GetError());
        return false;
    }
    initiateMap(game->maze);
    generateMazeLayout(game->maze);
    
    // Create camera
    game->camera = createCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!game->camera) {
        printf("Error: Failed to create camera\n");
        return false;
    }

    // Initialize projectiles
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        game->projectiles[i] = createProjectile(game->renderer);
        if (!game->projectiles[i]) {
            printf("Error: Failed to create projectile %d\n", i);
            return false;
        }
    }
    
    // Set window title based on connection type
    if (game->isNetworked) {
        if (game->isHost) {
            setWindowTitle(game, "Maze Mayhem - HOST");
            
            // For host, player ID is always 0
            // Spawn host (Player 0) in the Top-Left corner
            float corner_margin = 50.0f; // Margin from the absolute corner
            setPlayerPosition(game->localPlayer, corner_margin, corner_margin);
        } else {
            setWindowTitle(game, "Maze Mayhem - CLIENT");
        }
    } else {
        setWindowTitle(game, "Maze Mayhem - OFFLINE");
        setPlayerPosition(game->localPlayer, 400, 300);
    }

    game->isRunning = true;
    game->frameCounter = 0;
    
    return true;
}

void gameCoreRunFrame(GameContext *game) {
    static Uint32 lastTime = 0;
    static bool initialPosSet = false;

    // Initialize lastTime on first call
    if (lastTime == 0) {
        lastTime = SDL_GetTicks();
        initialPosSet = game->isHost; // Already set for host
    }

    // Handle time and delta time
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    
    // Handle events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game->isRunning = false;
        } else {
            handleInput(game, &event);
        }
    }
    
    // Update game state
    updateGame(game, deltaTime);
    updatePlayerRotation(game);
    
    // Network update
    if (game->isNetworked) {
        if (game->isHost) {
            hostTick(&game->netMgr, game);
        } else {
            clientTick(&game->netMgr, game);
            
            // If we're a client and we just got assigned a player ID, set initial position
            if (!initialPosSet && game->netMgr.localPlayerId != 0xFF) {
                float startX, startY;
                float corner_margin = 50.0f; // Margin from the absolute corner
                Uint8 id = game->netMgr.localPlayerId;

                if (id == 1) { // Top-Right
                    startX = WINDOW_WIDTH - PLAYERWIDTH - corner_margin;
                    startY = corner_margin;
                } else if (id == 2) { // Bottom-Left
                    startX = corner_margin;
                    startY = WINDOW_HEIGHT - (2 * PLAYERHEIGHT) - corner_margin;
                } else if (id == 3) { // Bottom-Right
                    startX = WINDOW_WIDTH - PLAYERWIDTH - corner_margin;
                    startY = WINDOW_HEIGHT - PLAYERHEIGHT - corner_margin;
                } else if (id == 4) { // Center
                    startX = WINDOW_WIDTH / 2.0f - PLAYERWIDTH / 2.0f;
                    startY = WINDOW_HEIGHT / 2.0f - PLAYERHEIGHT / 2.0f;
                } else {
                    // Fallback for unexpected IDs or if MAX_PLAYERS is increased
                    // Default to a slightly offset position
                    startX = 100.0f + (id * PLAYERWIDTH * 1.5f); // Spread them out a bit
                    startY = 100.0f;
                }

                setPlayerPosition(game->localPlayer, startX, startY);
                initialPosSet = true;
                
                // Update our local player index in the players array
                if (game->players[0] == game->localPlayer) {
                    game->players[0] = NULL;
                }
                game->players[game->netMgr.localPlayerId] = game->localPlayer;
                
                // Immediately send our position to other players
                SDL_Rect pos = getPlayerPosition(game->localPlayer);
                float angle = getPlayerAngle(game->localPlayer);
                sendPlayerPosition(&game->netMgr, (float)pos.x, (float)pos.y, angle);
            }
        }
        
        // Send position updates periodically to reduce network traffic
        game->frameCounter++;
        if (game->frameCounter >= UPDATE_RATE) {
            game->frameCounter = 0;
            
            // Only send if we have a valid player ID
            if (game->netMgr.localPlayerId != 0xFF) {
                SDL_Rect pos = getPlayerPosition(game->localPlayer);
                float angle = getPlayerAngle(game->localPlayer);
                sendPlayerPosition(&game->netMgr, (float)pos.x, (float)pos.y, angle);
            }
        }
    }
    
    // Render frame
    renderGame(game);
}

void handleInput(GameContext *game, SDL_Event *event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                game->isRunning = false;
                break;
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                movePlayerUp(game->localPlayer);
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                movePlayerDown(game->localPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                movePlayerLeft(game->localPlayer);
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                movePlayerRight(game->localPlayer);
                break;
            case SDL_SCANCODE_SPACE:
                spawnProjectile(game->projectiles, game->localPlayer);
                
                // Send shoot event over network
                if (game->isNetworked && game->netMgr.localPlayerId != 0xFF) {
                    SDL_Rect playerPos = getPlayerPosition(game->localPlayer);
                    float playerX = (float)(playerPos.x + playerPos.w/2);
                    float playerY = (float)(playerPos.y + playerPos.h/2);
                    float angle = getPlayerAngle(game->localPlayer);
                    sendPlayerShoot(&game->netMgr, playerX, playerY, angle);
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
                stopMovementVY(game->localPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_RIGHT:
                stopMovementVX(game->localPlayer);
                break;
            default:
                break;
        }
    }
}

void updateGame(GameContext *game, float deltaTime) {

    updatePlayer(game->localPlayer, deltaTime);
    
    SDL_Rect playerRect = getPlayerRect(game->localPlayer);
    bool collision = checkCollision(game->maze, playerRect);
    if (collision) {
        revertToPreviousPosition(game->localPlayer);
    }
    
    updateProjectileWithWallCollision(game->projectiles, game->maze, deltaTime);
    
    updateCamera(game->camera, game->localPlayer);
}

void updatePlayerRotation(GameContext *game) {
  
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
 
    SDL_Rect playerPos = getPlayerPosition(game->localPlayer);
    
    SDL_Rect screenPlayerPos = getWorldCoordinatesFromCamera(game->camera, playerPos);
    
   
    float playerCenterX = screenPlayerPos.x + screenPlayerPos.w / 2.0f;
    float playerCenterY = screenPlayerPos.y + screenPlayerPos.h / 2.0f;
    
    // Calculate angle between player center and mouse
    float deltaX = mouseX - playerCenterX;
    float deltaY = mouseY - playerCenterY;
    float radians = atan2f(deltaY, deltaX);
    float angle = radians * 180.0f / 3.14;
    
    // Update player angle
    setPlayerAngle(game->localPlayer, angle);
}

void renderGame(GameContext *game) {
  
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);
    
    drawMap(game->maze, game->camera, game->localPlayer);
    
   
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i] != NULL) {
            drawPlayer(game->players[i], game->camera);
        }
    }
    
   
    drawProjectile(game->projectiles, game->camera);

   
    SDL_RenderPresent(game->renderer);
}

void gameCoreShutdown(GameContext *game) {
  
    destroyProjectile(game->projectiles);

    if (game->camera) {
        destroyCamera(game->camera);
        game->camera = NULL;
    }

    // Clean up all players (except local player which is handled separately)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i] && game->players[i] != game->localPlayer) {
            destroyPlayer(game->players[i]);
            game->players[i] = NULL;
        }
    }

    if (game->localPlayer) {
        destroyPlayer(game->localPlayer);
        game->localPlayer = NULL;
    }

    if (game->maze) {
        destroyMaze(game->maze);
        game->maze = NULL;
    }
}

void gameOnNetworkMessage(GameContext *game, Uint8 type, Uint8 playerId, const void *data, int size) {
    if (playerId >= MAX_PLAYERS) return;
    
    switch (type) {
        case MSG_JOIN: {
            if (game->netMgr.localPlayerId == playerId) {
                if (game->players[0] == game->localPlayer) {
                    game->players[0] = NULL;
                }
                game->players[playerId] = game->localPlayer;
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
            if (game->players[playerId] == NULL && playerId != game->netMgr.localPlayerId) {
                game->players[playerId] = createPlayer(game->renderer);
                if (!game->players[playerId]) return;
                
                // positions  of players
                setPlayerPosition(game->players[playerId], x, y);
                setPlayerAngle(game->players[playerId], angle);
                
                printf("Created remote player with ID %d\n", playerId);
            }
            
            // Skip updating if it's our local player
            if (playerId != game->netMgr.localPlayerId) {
                // updatera spelaren
                setPlayerPosition(game->players[playerId], x, y);
                setPlayerAngle(game->players[playerId], angle);
            }
            break;
        }
        case MSG_SHOOT: {
            if (size < 3.0 * sizeof(float)) return;

            if (playerId == game->netMgr.localPlayerId) return;
            
            // Ensure remote player exists
            if (game->players[playerId] == NULL) return;
            
            // Spawn projectile for remote player
            spawnProjectile(game->projectiles, game->players[playerId]);
            break;
        }
        case MSG_LEAVE: {
            
                if (playerId != game->netMgr.localPlayerId && game->players[playerId]) {
                destroyPlayer(game->players[playerId]);
                game->players[playerId] = NULL;
            }
            break;
        }
    }
} 