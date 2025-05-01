#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include <math.h>     
#include <string.h>
#include "../include/constants.h"
#include "../include/player.h"
#include "../include/camera.h"
#include "../include/maze.h"
#include "../include/projectile.h"
#include "../include/network.h"

#define NAWID
#define DEFAULT_PORT 7777
#define UPDATE_RATE 10  // Send position updates every 10 frames

struct game {
    bool isRunning;
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Player *pPlayer;
    Player *pPlayers[MAX_PLAYERS]; // Array of all players (local and remote)
    Camera *pCamera;
    Maze *pMaze;
    SDL_Texture* tileMapTexture;
    SDL_Surface* tileMapSurface;
    int tileMap[TILE_WIDTH][TILE_HEIGHT];
    Projectile *pProjectile[MAX_PROJECTILES];
    
    // Network related
    NetMgr netMgr;
    bool isNetworked;
    bool isHost;
    int frameCounter;
};
typedef struct game Game;

bool initiateGame(Game *pGame, bool isHost, const char *joinIP);
void closeGame(Game *pGame);
void run(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void updateGame(Game *pGame, float deltaTime);
void renderGame(Game *pGame);
void updatePlayerRotation(Game *pGame);
// Helper function to set window title with connection info
void setWindowTitle(Game *pGame, const char *title);

// Network handling function
void handleNetworkData(Uint8 type, Uint8 playerId, const void *data, int size);

// Global game instance for network callback
Game *g_Game = NULL;

int main(int argc, char** argv)
{
    bool isHost = false;
    const char *joinIP = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0) {
            isHost = true;
        } else if (strcmp(argv[i], "--join") == 0 && i + 1 < argc) {
            joinIP = argv[i + 1];
            i++; // Skip next argument (IP address)
        }
    }
    
    Game g = {0};
    g_Game = &g; // Set global pointer for network callback
    
    if(!initiateGame(&g, isHost, joinIP)) return 1;
    run(&g);
    closeGame(&g);
    return 0;
}

bool initiateGame(Game *pGame, bool isHost, const char *joinIP)
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("SDL Initialization Error: %s\n", SDL_GetError());
        return false;
    }
    
    pGame->pWindow = SDL_CreateWindow("Maze Mayhem", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
    if (!pGame->pWindow)
    {
        printf("Window Creation Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return false;
    }
    
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!pGame->pRenderer)
    {
        printf("Renderer Creation Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return false;    
    }

    // Initialize players array
    for (int i = 0; i < MAX_PLAYERS; i++) {
        pGame->pPlayers[i] = NULL;
    }

    // Create local player
    pGame->pPlayer = createPlayer(pGame->pRenderer);
    if(!pGame->pPlayer){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return false;
    }
    
    // Store local player in player array (will be updated when ID is assigned)
    pGame->pPlayers[0] = pGame->pPlayer;

    pGame->pMaze = createMaze(pGame->pRenderer, pGame->tileMapTexture, pGame->tileMapSurface);
    if(!pGame->pMaze){
        printf("Maze Creation Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return false;
    }
    initiateMap(pGame->pMaze);
    generateMazeLayout(pGame->pMaze);
    
    pGame->pCamera = createCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!pGame->pCamera){
        printf("Error: Failed to create camera\n");
        closeGame(pGame);
        return false;
    }

    // Initialize projectiles
    for(int i = 0; i < MAX_PROJECTILES; i++){
        pGame->pProjectile[i] = createProjectile(pGame->pRenderer);
        if(!pGame->pProjectile[i]){
            printf("Error: Failed to create projectile %d\n", i);
            closeGame(pGame);
            return false;
        }
    }
    
    // Initialize networking if required
    pGame->isNetworked = isHost || joinIP != NULL;
    pGame->isHost = isHost;
    pGame->frameCounter = 0;
    
    if (pGame->isNetworked) {
        // Initialize SDL_net
        if (!netInit()) {
            printf("Failed to initialize SDL_net: %s\n", SDLNet_GetError());
            closeGame(pGame);
            return false;
        }
        
        // Start host or connect to server
        if (isHost) {
            if (!hostStart(&pGame->netMgr, DEFAULT_PORT)) {
                printf("Failed to start host: %s\n", SDLNet_GetError());
                closeGame(pGame);
                return false;
            }
            
            // For host, player ID is always 0
            setWindowTitle(pGame, "Maze Mayhem - HOST");
            
            // Set host player starting position (center)
            setPlayerPosition(pGame->pPlayer, 400, 300);
        } else if (joinIP) {
            if (!clientConnect(&pGame->netMgr, joinIP, DEFAULT_PORT)) {
                printf("Failed to connect to host: %s\n", SDLNet_GetError());
                closeGame(pGame);
                return false;
            }
            
            setWindowTitle(pGame, "Maze Mayhem - CLIENT");
            
            // Client will get its ID assigned by server, and position will be updated in run loop
        }
    }

    return true;
}

// Helper function to set window title with connection info
void setWindowTitle(Game *pGame, const char *title) {
    if (pGame->pWindow) {
        SDL_SetWindowTitle(pGame->pWindow, title);
    }
}

void run(Game *pGame)
{
    pGame->isRunning = true;
    SDL_Event event;

    Uint32 lastTime = SDL_GetTicks(); //time of the first frame since SDl init
    Uint32 currentTime; //time when the current frame started
    float deltaTime; //move player by pixels in second instead of by frames in second
    
    // Flag to track if we've set the initial position for the client
    bool initialPosSet = pGame->isHost; // Already set for host

    while(pGame->isRunning)
    {
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; //convert ms to seconds
        lastTime = currentTime;
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                pGame->isRunning = false;
            }
            else
            {
                handleInput(pGame, &event);
            }
        }
        updateGame(pGame, deltaTime);
        updatePlayerRotation(pGame);
        
        // Network update
        if (pGame->isNetworked) {
            if (pGame->isHost) {
                hostTick(&pGame->netMgr);
            } else {
                clientTick(&pGame->netMgr);
                
                // If we're a client and we just got assigned a player ID, set initial position
                if (!initialPosSet && pGame->netMgr.localPlayerId != 0xFF) {
                    // Set different starting positions based on player ID
                    float startX = 300.0f + (pGame->netMgr.localPlayerId * 100);
                    float startY = 200.0f + (pGame->netMgr.localPlayerId * 80);
                    setPlayerPosition(pGame->pPlayer, startX, startY);
                    initialPosSet = true;
                    
                    // Update our local player index in the players array
                    if (pGame->pPlayers[0] == pGame->pPlayer) {
                        pGame->pPlayers[0] = NULL;
                    }
                    pGame->pPlayers[pGame->netMgr.localPlayerId] = pGame->pPlayer;
                    
                    // Immediately send our position to other players
                    SDL_Rect pos = getPlayerPosition(pGame->pPlayer);
                    float angle = getPlayerAngle(pGame->pPlayer);
                    sendPlayerPosition(&pGame->netMgr, (float)pos.x, (float)pos.y, angle);
                }
            }
            
            // Send position updates periodically to reduce network traffic
            pGame->frameCounter++;
            if (pGame->frameCounter >= UPDATE_RATE) {
                pGame->frameCounter = 0;
                
                // Only send if we have a valid player ID
                if (pGame->netMgr.localPlayerId != 0xFF) {
                    SDL_Rect pos = getPlayerPosition(pGame->pPlayer);
                    float angle = getPlayerAngle(pGame->pPlayer);
                    sendPlayerPosition(&pGame->netMgr, (float)pos.x, (float)pos.y, angle);
                }
            }
        }
        
        renderGame(pGame);
    }
}

void handleInput(Game *pGame, SDL_Event *pEvent)
{
    if(pEvent->type == SDL_KEYDOWN)
    {
        switch(pEvent->key.keysym.scancode)
        {
            case SDL_SCANCODE_ESCAPE:
                pGame->isRunning = false;
                break;
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                movePlayerUp(pGame->pPlayer);
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                movePlayerDown(pGame->pPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                movePlayerLeft(pGame->pPlayer);
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                movePlayerRight(pGame->pPlayer);
                break;
            case SDL_SCANCODE_SPACE:
                spawnProjectile(pGame->pProjectile, pGame->pPlayer);
                
                // Send shoot event over network
                if (pGame->isNetworked && pGame->netMgr.localPlayerId != 0xFF) {
                    // Instead of getting gun tip position, use player position and angle
                    SDL_Rect playerPos = getPlayerPosition(pGame->pPlayer);
                    float playerX = (float)(playerPos.x + playerPos.w/2);
                    float playerY = (float)(playerPos.y + playerPos.h/2);
                    float angle = getPlayerAngle(pGame->pPlayer);
                    sendPlayerShoot(&pGame->netMgr, playerX, playerY, angle);
                }
                break;
            default:
                break;
        }
    }
    else if(pEvent->type == SDL_KEYUP)
    {
        switch (pEvent->key.keysym.scancode)
        {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_DOWN:
                stopMovementVY(pGame->pPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_RIGHT:
                stopMovementVX(pGame->pPlayer);
                break;
            default:
                break;
        }
    }
}

void updateGame(Game *pGame, float deltaTime)
{
    // Update player position
    updatePlayer(pGame->pPlayer, deltaTime);
    
    // Check collision and revert if needed
    SDL_Rect playerRect = getPlayerRect(pGame->pPlayer);
    bool collision = checkCollision(pGame->pMaze, playerRect);
    if (collision) {
        revertToPreviousPosition(pGame->pPlayer);
    }
    
    // Update projectiles with wall collision detection
    updateProjectileWithWallCollision(pGame->pProjectile, pGame->pMaze, deltaTime);
    
    // Update camera after final player position is determined
    updateCamera(pGame->pCamera, pGame->pPlayer);
}

void updatePlayerRotation(Game *pGame)
{
    // Get mouse position in screen coordinates
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    // Get player position in world coordinates
    SDL_Rect playerPos = getPlayerPosition(pGame->pPlayer);
    
    // Convert player position to screen coordinates using camera
    SDL_Rect screenPlayerPos = getWorldCoordinatesFromCamera(pGame->pCamera, playerPos);
    
    // Calculate player center in screen coordinates
    float playerCenterX = screenPlayerPos.x + screenPlayerPos.w / 2.0f;
    float playerCenterY = screenPlayerPos.y + screenPlayerPos.h / 2.0f;
    
    // Calculate angle between player center and mouse
    float deltaX = mouseX - playerCenterX;
    float deltaY = mouseY - playerCenterY;
    float radians = atan2f(deltaY, deltaX);
    float angle = radians * 180.0f / 3.14;
    
    // Update player angle
    setPlayerAngle(pGame->pPlayer, angle);
}

void renderGame(Game *pGame)
{
    // Clear with a dark background
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);
    
    // Draw background (now a solid color instead of texture)
    drawMap(pGame->pMaze, pGame->pCamera, pGame->pPlayer);
    
    // Draw all players
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (pGame->pPlayers[i] != NULL) {
            drawPlayer(pGame->pPlayers[i], pGame->pCamera);
        }
    }
    
    // Draw projectiles
    drawProjectile(pGame->pProjectile, pGame->pCamera);

    // Present the rendered frame
    SDL_RenderPresent(pGame->pRenderer);
}

void closeGame(Game *pGame)
{
    // Clean up network resources
    if (pGame->isNetworked) {
        if (pGame->isHost) {
            // Close all client connections
            for (int i = 0; i < pGame->netMgr.peerCount; i++) {
                if (pGame->netMgr.peers[i]) {
                    SDLNet_TCP_Close(pGame->netMgr.peers[i]);
                }
            }
            
            // Close server socket
            if (pGame->netMgr.server) {
                SDLNet_TCP_Close(pGame->netMgr.server);
            }
        } else {
            // Close client connection
            if (pGame->netMgr.client) {
                SDLNet_TCP_Close(pGame->netMgr.client);
            }
        }
        
        // Free socket set
        if (pGame->netMgr.set) {
            SDLNet_FreeSocketSet(pGame->netMgr.set);
        }
        
        netShutdown();
    }

    // Clean up projectiles
    destroyProjectile(pGame->pProjectile);

    if(pGame->pCamera){
        destroyCamera(pGame->pCamera);
        pGame->pCamera = NULL;
    }

    // Clean up all players (except local player which is handled separately)
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (pGame->pPlayers[i] && pGame->pPlayers[i] != pGame->pPlayer) {
            destroyPlayer(pGame->pPlayers[i]);
            pGame->pPlayers[i] = NULL;
        }
    }

    if(pGame->pPlayer){
        destroyPlayer(pGame->pPlayer);
        pGame->pPlayer = NULL;
    }

    if(pGame->pMaze){
        destroyMaze(pGame->pMaze);
        pGame->pMaze = NULL;
    }

    if(pGame->pRenderer){
        SDL_DestroyRenderer(pGame->pRenderer);
        pGame->pRenderer = NULL;
    }

    if(pGame->pWindow){
        SDL_DestroyWindow(pGame->pWindow);
        pGame->pWindow = NULL;
    }

    SDL_Quit();
}

// Network data handler implementation
void handleNetworkData(Uint8 type, Uint8 playerId, const void *data, int size) {
    if (!g_Game) return;
    
    // Ensure player ID is valid
    if (playerId >= MAX_PLAYERS) return;
    
    switch (type) {
        case MSG_JOIN: {
            // Server assigned us a player ID
            if (g_Game->netMgr.localPlayerId == playerId) {
                // This is for the local player, update player array
                if (g_Game->pPlayers[0] == g_Game->pPlayer) {
                    g_Game->pPlayers[0] = NULL;
                }
                g_Game->pPlayers[playerId] = g_Game->pPlayer;
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
            if (g_Game->pPlayers[playerId] == NULL && playerId != g_Game->netMgr.localPlayerId) {
                g_Game->pPlayers[playerId] = createPlayer(g_Game->pRenderer);
                if (!g_Game->pPlayers[playerId]) return;
                
                // Set the initial position
                setPlayerPosition(g_Game->pPlayers[playerId], x, y);
                setPlayerAngle(g_Game->pPlayers[playerId], angle);
                
                printf("Created remote player with ID %d\n", playerId);
            }
            
            // Skip updating if it's our local player
            if (playerId != g_Game->netMgr.localPlayerId) {
                // Update remote player position and angle
                setPlayerPosition(g_Game->pPlayers[playerId], x, y);
                setPlayerAngle(g_Game->pPlayers[playerId], angle);
            }
            break;
        }
        case MSG_SHOOT: {
            // Ensure data contains shoot information
            if ((unsigned int)size < sizeof(float) * 3) return;
            
            // Skip if it's our local player (we already spawned the projectile)
            if (playerId == g_Game->netMgr.localPlayerId) return;
            
            // Ensure remote player exists
            if (g_Game->pPlayers[playerId] == NULL) return;
            
            // Spawn projectile for remote player
            spawnProjectile(g_Game->pProjectile, g_Game->pPlayers[playerId]);
            break;
        }
        case MSG_LEAVE: {
            // Handle player disconnection
            if (playerId != g_Game->netMgr.localPlayerId && g_Game->pPlayers[playerId]) {
                destroyPlayer(g_Game->pPlayers[playerId]);
                g_Game->pPlayers[playerId] = NULL;
            }
            break;
        }
    }
}
