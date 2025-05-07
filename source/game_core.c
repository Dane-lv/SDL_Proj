#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
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
#include "../include/audio_manager.h"

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
    
    // Initialize text renderer
    ctx->textRenderer = createTextRenderer(ctx->renderer);
    if (!ctx->textRenderer) {
        printf("Error: Failed to create text renderer\n");
        // Continue anyway - we'll just not render text
    }
    
    // Initialize audio system
    if (!audioInit(&ctx->audioManager)) {
        printf("Warning: Failed to initialize audio system\n");
        // Continue anyway - we'll just not have sound
    } else {
        // Load and play background music
        if (loadBackgroundMusic(&ctx->audioManager, "resources/music/background music.mp3")) {
            playBackgroundMusic(&ctx->audioManager);
        } else {
            printf("Warning: Failed to load background music\n");
        }
        
        // Load game over sound
        if (!loadGameOverSound(&ctx->audioManager, "resources/sfx/GameOver.mp3")) {
            printf("Warning: Failed to load game over sound\n");
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
                // Don't shoot if player is eliminated
                if (!isPlayerEliminated(ctx->localPlayer)) {
                    spawnProjectile(ctx->projectiles, ctx->localPlayer);
                    
                    // Send shoot event over network
                    if (ctx->isNetworked && ctx->netMgr.localPlayerId != 0xFF) {
                        // Instead of getting gun tip position, use player position and angle
                        SDL_Rect playerPos = getPlayerPosition(ctx->localPlayer);
                        float playerX = (float)(playerPos.x + playerPos.w/2);
                        float playerY = (float)(playerPos.y + playerPos.h/2);
                        float angle = getPlayerAngle(ctx->localPlayer);
                        sendPlayerShoot(&ctx->netMgr, playerX, playerY, angle);
                    }
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
    // Update player position (this already has an internal check for isEliminated)
    updatePlayer(ctx->localPlayer, deltaTime);
    
    // Check collision and revert if needed
    SDL_Rect playerRect = getPlayerRect(ctx->localPlayer);
    bool collision = checkCollision(ctx->maze, playerRect);
    if (collision) {
        revertToPreviousPosition(ctx->localPlayer);
    }
    
    // Update projectiles with wall collision detection
    updateProjectileWithWallCollision(ctx->projectiles, ctx->maze, deltaTime);
    
    // Check for projectile collisions with players
    checkProjectilePlayerCollisions(ctx);
    
    // Update camera after final player position is determined
    updateCamera(ctx->camera, ctx->localPlayer);
}

// Check for collisions between projectiles and players
void checkProjectilePlayerCollisions(GameContext *ctx) {
    // Check each active projectile
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (isProjectileActive(ctx->projectiles[i])) {
            SDL_Rect projRect = getProjectileRect(ctx->projectiles[i]);
            
            // Check collision with each player
            for (int j = 0; j < MAX_PLAYERS; j++) {
                if (ctx->players[j] != NULL && !isPlayerEliminated(ctx->players[j])) {
                    SDL_Rect playerRect = getPlayerRect(ctx->players[j]);
                    
                    // Skip collision check for the first few frames after a projectile is spawned
                    // to prevent self-elimination
                    if (getProjectileAge(ctx->projectiles[i]) < 0.1f) {
                        continue;
                    }
                    
                    // Check if projectile intersects with player
                    if (SDL_HasIntersection(&projRect, &playerRect)) {
                        // Eliminate the player
                        eliminatePlayer(ctx->players[j]);
                        
                        // Play game over sound if local player is eliminated
                        if (ctx->players[j] == ctx->localPlayer) {
                            playGameOverSound(&ctx->audioManager);
                        }
                        
                        // If this is a networked game, send elimination message
                        if (ctx->isNetworked && ctx->netMgr.localPlayerId != 0xFF) {
                            // If the eliminated player is local, inform others
                            if (ctx->players[j] == ctx->localPlayer) {
                                sendPlayerEliminated(&ctx->netMgr);
                            }
                        }
                    }
                }
            }
        }
    }
}

void updatePlayerRotation(GameContext *ctx) {
    // Get mouse position in screen coordinates
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    // Get player position in world coordinates
    SDL_Rect playerPos = getPlayerPosition(ctx->localPlayer);
    
    // Convert player position to screen coordinates using camera
    SDL_Rect screenPlayerPos = getWorldCoordinatesFromCamera(ctx->camera, playerPos);
    
    // Calculate player center in screen coordinates
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
    // Clear with a dark background
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);
    
    // Draw background (now a solid color instead of texture)
    drawMap(ctx->maze, ctx->camera, ctx->localPlayer);
    
    // Draw all players (only if not eliminated)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (ctx->players[i] != NULL && !isPlayerEliminated(ctx->players[i])) {
            drawPlayer(ctx->players[i], ctx->camera);
        }
    }
    
    // Draw projectiles
    drawProjectile(ctx->projectiles, ctx->camera);

    // If local player is eliminated, render a semi-transparent black overlay with GAME OVER text
    if (isPlayerEliminated(ctx->localPlayer)) {
        // Set the renderer to 80% black
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 204); // 80% opacity (204/255)
        SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
        
        // Create a rect covering the entire screen
        SDL_Rect fullScreenRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(ctx->renderer, &fullScreenRect);
        
        // Render "GAME OVER" text using the text renderer
        if (ctx->textRenderer) {
            renderGameOverText(ctx->textRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
        }
        
        // Reset blend mode
        SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
    }

    // Present the rendered frame
    SDL_RenderPresent(ctx->renderer);
}

void gameCoreShutdown(GameContext *ctx) {
    // Clean up text renderer
    if (ctx->textRenderer) {
        destroyTextRenderer(ctx->textRenderer);
        ctx->textRenderer = NULL;
    }
    TTF_Quit();
    
    // Clean up projectiles
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

    // Clean up audio resources
    audioCleanup(&ctx->audioManager);
}

void gameOnNetworkMessage(GameContext *ctx, Uint8 type, Uint8 playerId, const void *data, int size) {
    // Ensure player ID is valid
    if (playerId >= MAX_PLAYERS) return;
    
    switch (type) {
        case MSG_JOIN: {
            // Server assigned us a player ID
            if (ctx->netMgr.localPlayerId == playerId) {
                // This is for the local player, update player array
                if (ctx->players[0] == ctx->localPlayer) {
                    ctx->players[0] = NULL;
                }
                ctx->players[playerId] = ctx->localPlayer;
            }
            break;
        }
        case MSG_POS: {
            // Ensure data contains position information
            if ((unsigned int)size < sizeof(float) * 3) return;
            
            float x = *((float*)data);
            float y = *((float*)data + 1);
            float angle = *((float*)data + 2);
            
            // Create remote player if it doesn't exist
            if (ctx->players[playerId] == NULL && playerId != ctx->netMgr.localPlayerId) {
                ctx->players[playerId] = createPlayer(ctx->renderer);
                if (!ctx->players[playerId]) return;
                
                // Set the initial position
                setPlayerPosition(ctx->players[playerId], x, y);
                setPlayerAngle(ctx->players[playerId], angle);
                
                printf("Created remote player with ID %d\n", playerId);
            }
            
            // Skip updating if it's our local player
            if (playerId != ctx->netMgr.localPlayerId) {
                // Update remote player position and angle
                setPlayerPosition(ctx->players[playerId], x, y);
                setPlayerAngle(ctx->players[playerId], angle);
            }
            break;
        }
        case MSG_SHOOT: {
            // Ensure data contains shoot information
            if ((unsigned int)size < sizeof(float) * 3) return;
            
            // Skip if it's our local player (we already spawned the projectile)
            if (playerId == ctx->netMgr.localPlayerId) return;
            
            // Ensure remote player exists
            if (ctx->players[playerId] == NULL) return;
            
            // Spawn projectile for remote player
            spawnProjectile(ctx->projectiles, ctx->players[playerId]);
            break;
        }
        case MSG_ELIMINATED: {
            // Handle player elimination
            if (ctx->players[playerId] != NULL) {
                eliminatePlayer(ctx->players[playerId]);
                
                // Play game over sound if local player is eliminated
                if (playerId == ctx->netMgr.localPlayerId) {
                    playGameOverSound(&ctx->audioManager);
                }
                
                printf("Player %d has been eliminated!\n", playerId);
            }
            break;
        }
        case MSG_LEAVE: {
            // Handle player disconnection
            if (playerId != ctx->netMgr.localPlayerId && ctx->players[playerId]) {
                destroyPlayer(ctx->players[playerId]);
                ctx->players[playerId] = NULL;
            }
            break;
        }
    }
} 