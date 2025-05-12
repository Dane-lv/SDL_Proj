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

// Function declarations
static void setWindowTitle(GameContext *game, const char *title);
void initDeathScreen(GameContext *game);
void renderDeathScreen(GameContext *game);
void enableSpectateMode(GameContext *game);
void checkPlayerProjectileCollisions(GameContext *game);

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
    
    // Initialize audio manager (only if not already initialized)
    if (!game->audioManager) {
        game->audioManager = createAudioManager();
        if (!game->audioManager) {
            printf("Warning: Failed to create audio manager. Game will continue without sound.\n");
            // Continue without audio
        } else {
            // Start playing background music in a loop
            playBackgroundMusic(game->audioManager);
        }
    }
    
    // Initialize death screen and spectate mode
    game->isSpectating = false;
    game->showDeathScreen = false;
    
    // Create "YOU DIED" text texture
    SDL_Surface *textSurface = TTF_RenderText_Blended(
        TTF_OpenFont("resources/font.ttf", 64),
        "YOU DIED",
        (SDL_Color){255, 0, 0, 255} // Red
    );
    if (textSurface) {
        game->fontTexture = SDL_CreateTextureFromSurface(game->renderer, textSurface);
        SDL_FreeSurface(textSurface);
    } else {
        game->fontTexture = NULL;
        printf("Warning: Failed to create font texture\n");
    }
    
    // Initialize spectate button rectangle
    initDeathScreen(game);
    
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
    // Death screen interaction
    if (!isPlayerAlive(game->localPlayer) && game->showDeathScreen) {
        if (event->type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            
            // Check if spectate button was clicked
            if (mouseX >= game->spectateButtonRect.x && 
                mouseX <= game->spectateButtonRect.x + game->spectateButtonRect.w &&
                mouseY >= game->spectateButtonRect.y && 
                mouseY <= game->spectateButtonRect.y + game->spectateButtonRect.h) {
                
                // Enable spectate mode
                enableSpectateMode(game);
            }
        }
        return; // Don't process other input while on death screen
    }
    
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
                // Only allow shooting if player is alive
                if (isPlayerAlive(game->localPlayer)) {
                    // Spawn the projectile and get its ID
                    int projectileId = spawnProjectile(game->projectiles, game->localPlayer);
                    
                    // If projectile was spawned successfully
                    if (projectileId >= 0 && game->isNetworked) {
                        SDL_Rect playerPos = getPlayerPosition(game->localPlayer);
                        float angle = getPlayerAngle(game->localPlayer);
                        float x = playerPos.x + playerPos.w / 2.0f;
                        float y = playerPos.y + playerPos.h / 2.0f;
                        
                        // Factor in angle to place projectile outside the player
                        float radians = angle * M_PI / 180.0f;
                        float offsetDistance = 5.0f; // Minimal offset, matching spawn function
                        x += cosf(radians) * offsetDistance;
                        y += sinf(radians) * offsetDistance;
                        
                        // Send projectile data with projectile ID
                        sendPlayerShoot(&game->netMgr, x, y, angle, projectileId);
                        
                        // Also send a position update immediately after shooting
                        // This ensures spectators can still see the player
                        sendPlayerPosition(&game->netMgr, (float)playerPos.x, (float)playerPos.y, angle);
                    }
                }
                break;
        }
    } else if (event->type == SDL_KEYUP) {
        switch (event->key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                stopMovementVY(game->localPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                stopMovementVX(game->localPlayer);
                break;
        }
    }
}

void updateGame(GameContext *game, float deltaTime) {
    // Update local player
    updatePlayer(game->localPlayer, deltaTime);
    
    // Prevent player from leaving the world boundaries
    SDL_Rect playerRect = getPlayerPosition(game->localPlayer);
    if (playerRect.x < 0 || playerRect.x + playerRect.w > WORLD_WIDTH ||
        playerRect.y < 0 || playerRect.y + playerRect.h > WORLD_HEIGHT) {
        revertToPreviousPosition(game->localPlayer);
    }
    
    // Check for wall collisions
    if (checkCollision(game->maze, playerRect)) {
        revertToPreviousPosition(game->localPlayer);
    }
    
    // Update projectiles with wall collision
    updateProjectileWithWallCollision(game->projectiles, game->maze, deltaTime);
    
    // Check for projectile-player collisions
    checkPlayerProjectileCollisions(game);
    
    // Update camera based on spectate mode
    if (game->isSpectating) {
        // In spectate mode, keep the camera centered on the maze
        float mazeCenterX = (TILE_WIDTH * TILE_SIZE) / 2.0f;
        float mazeCenterY = (TILE_HEIGHT * TILE_SIZE) / 2.0f;
        setCameraPosition(game->camera, mazeCenterX, mazeCenterY);
    } else {
        // Normal camera following player
        updateCamera(game->camera, game->localPlayer);
    }
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
    SDL_SetRenderDrawColor(game->renderer, 10, 10, 10, 255);
    SDL_RenderClear(game->renderer);
    
    // Render the maze walls (pass spectate mode flag)
    drawMap(game->maze, game->camera, game->localPlayer, game->isSpectating);
    
    // Render all players and projectiles
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i]) {
            drawPlayer(game->players[i], game->camera);
        }
    }
    
    // Draw projectiles
    drawProjectile(game->projectiles, game->camera);
    
    // Render death screen if player is dead and not in spectate mode
    if (!isPlayerAlive(game->localPlayer) && game->showDeathScreen) {
        renderDeathScreen(game);
    }
    
    SDL_RenderPresent(game->renderer);
}

void gameCoreShutdown(GameContext *game) {
    // Clean up network resources if networked game
    if (game->isNetworked) {
        netShutdown();
    }
    
    // Destroy all projectiles
    destroyProjectile(game->projectiles);
    
    // Destroy local player
    destroyPlayer(game->localPlayer);
    
    // Destroy other players (in networked games)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i] && game->players[i] != game->localPlayer) {
            destroyPlayer(game->players[i]);
        }
    }
    
    // Destroy maze, camera and font texture
    destroyMaze(game->maze);
    destroyCamera(game->camera);
    
    // Destroy font texture
    if (game->fontTexture) {
        SDL_DestroyTexture(game->fontTexture);
        game->fontTexture = NULL;
    }
    
    // Destroy audio manager
    if (game->audioManager) {
        destroyAudioManager(game->audioManager);
        game->audioManager = NULL;
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
            if (size < 3.0 * sizeof(float) + sizeof(int)) return;

            if (playerId == game->netMgr.localPlayerId) return;
            
            // Ensure remote player exists
            if (game->players[playerId] == NULL) return;
            
            // Get projectile ID - properly cast data pointer to access int
            const float* posData = (const float*)data;
            int projectileId = *((const int*)(posData + 3));
            
            // Check if projectile ID is valid
            if (projectileId < 0 || projectileId >= MAX_PROJECTILES) return;
            
            // Get position and angle data
            float x = posData[0];
            float y = posData[1];
            float angle = posData[2];
            
            // Convert angle to radians
            float radians = angle * M_PI / 180.0f;
            
            // Set projectile properties using accessor functions
            setProjectileActive(game->projectiles[projectileId], true);
            setProjectileOwner(game->projectiles[projectileId], game->players[playerId]);
            setProjectilePosition(game->projectiles[projectileId], x, y);
            setProjectileVelocity(
                game->projectiles[projectileId], 
                cosf(radians) * PROJSPEED, 
                sinf(radians) * PROJSPEED
            );
            setProjectileDuration(game->projectiles[projectileId], 3.0f);  // 3 seconds duration
            
            break;
        }
        case MSG_LEAVE: {
            
                if (playerId != game->netMgr.localPlayerId && game->players[playerId]) {
                destroyPlayer(game->players[playerId]);
                game->players[playerId] = NULL;
            }
            break;
        }
        case MSG_DEATH: {
            if (size < sizeof(Uint8)) return;
            
            // Get the player who died
            Player* deadPlayer = game->players[playerId];
            if (!deadPlayer) return;
            
            // Only show death screen if it's the local player
            if (deadPlayer == game->localPlayer) {
                game->showDeathScreen = true;
                
                // Play death sound
                if (game->audioManager) {
                    playDeathSound(game->audioManager);
                }
            }
            
            // Kill the player
            killPlayer(deadPlayer);
            
            printf("Player %d has died\n", playerId);
            break;
        }
    }
}

// Check for collisions between projectiles and players
void checkPlayerProjectileCollisions(GameContext *game) {
    // For each projectile, check collision with all players
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (game->projectiles[i] && isProjectileActive(game->projectiles[i])) {
            
            // Check collision with the local player
            if (isPlayerAlive(game->localPlayer) && 
                checkProjectilePlayerCollision(game->projectiles[i], game->localPlayer)) {
                // Kill player
                killPlayer(game->localPlayer);
                // Deactivate projectile
                deactivateProjectile(game->projectiles[i]);
                // Show death screen
                game->showDeathScreen = true;
                
                // Play death sound
                if (game->audioManager) {
                    playDeathSound(game->audioManager);
                }
                
                // Send death message if networked game
                if (game->isNetworked) {
                    // Get the player ID who killed this player
                    Player* killerPlayer = getProjectileOwner(game->projectiles[i]);
                    Uint8 killerPlayerId = 0xFF; // Default to invalid ID
                    
                    // Find the player ID of the killer
                    for (int j = 0; j < MAX_PLAYERS; j++) {
                        if (game->players[j] == killerPlayer) {
                            killerPlayerId = j;
                            break;
                        }
                    }
                    
                    // Send death notification
                    sendPlayerDeath(&game->netMgr, killerPlayerId);
                }
            }
            
            // Check collision with other players in networked games
            if (game->isNetworked) {
                for (int j = 0; j < MAX_PLAYERS; j++) {
                    if (game->players[j] && game->players[j] != game->localPlayer && 
                        isPlayerAlive(game->players[j]) && 
                        checkProjectilePlayerCollision(game->projectiles[i], game->players[j])) {
                        // Kill the player
                        killPlayer(game->players[j]);
                        // Deactivate projectile
                        deactivateProjectile(game->projectiles[i]);
                        
                        // We don't show death screen for other players
                        // We also don't need to send network messages for remote player collisions 
                        // as those will be handled by the player's own client
                    }
                }
            }
        }
    }
}

// Initialize the death screen
void initDeathScreen(GameContext *game) {
    int buttonWidth = 200;
    int buttonHeight = 50;
    
    // Center the spectate button on screen
    game->spectateButtonRect.x = (WINDOW_WIDTH / 2) - (buttonWidth / 2);
    game->spectateButtonRect.y = (WINDOW_HEIGHT / 2) + 50; // Position below "YOU DIED" text
    game->spectateButtonRect.w = buttonWidth;
    game->spectateButtonRect.h = buttonHeight;
}

// Render the death screen
void renderDeathScreen(GameContext *game) {
    SDL_Renderer *renderer = game->renderer;
    
    // Draw semi-transparent black overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Semi-transparent black
    SDL_Rect fullScreen = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &fullScreen);
    
    // Draw "YOU DIED" text
    if (game->fontTexture) {
        SDL_Rect textRect = {
            (WINDOW_WIDTH / 2) - 150, // Center horizontally
            (WINDOW_HEIGHT / 2) - 50, // Position at center
            300, // Width of text
            80   // Height of text
        };
        
        // Set text to be tinted red
        SDL_SetTextureColorMod(game->fontTexture, 255, 0, 0);
        SDL_RenderCopy(renderer, game->fontTexture, NULL, &textRect);
    }
    
    // Draw spectate button
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Gray button
    SDL_RenderFillRect(renderer, &game->spectateButtonRect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Light gray border
    SDL_RenderDrawRect(renderer, &game->spectateButtonRect);
    
    // Render "SPECTATE" text on the button
    TTF_Font* buttonFont = TTF_OpenFont("resources/font.ttf", 28);
    if (buttonFont) {
        SDL_Color textColor = {255, 255, 255, 255}; // White text
        SDL_Surface* buttonTextSurface = TTF_RenderText_Blended(buttonFont, "SPECTATE", textColor);
        if (buttonTextSurface) {
            SDL_Texture* buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
            if (buttonTextTexture) {
                // Center the text on the button
                SDL_Rect buttonTextRect = {
                    game->spectateButtonRect.x + (game->spectateButtonRect.w - buttonTextSurface->w) / 2,
                    game->spectateButtonRect.y + (game->spectateButtonRect.h - buttonTextSurface->h) / 2,
                    buttonTextSurface->w,
                    buttonTextSurface->h
                };
                SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect);
                SDL_DestroyTexture(buttonTextTexture);
            }
            SDL_FreeSurface(buttonTextSurface);
        }
        TTF_CloseFont(buttonFont);
    }
}

// Enable spectate mode
void enableSpectateMode(GameContext *game) {
    game->isSpectating = true;
    game->showDeathScreen = false;
    
    // Set camera to spectate mode
    setCameraSpectateMode(game->camera, true);
    
    // Center camera on the maze (not the world)
    // Calculate maze center: (TILE_WIDTH * TILE_SIZE / 2, TILE_HEIGHT * TILE_SIZE / 2)
    float mazeCenterX = (TILE_WIDTH * TILE_SIZE) / 2.0f;
    float mazeCenterY = (TILE_HEIGHT * TILE_SIZE) / 2.0f;
    setCameraPosition(game->camera, mazeCenterX, mazeCenterY);
} 