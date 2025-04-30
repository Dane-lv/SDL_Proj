#include <stdio.h>
#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>

#include "../include/constants.h"
#include "../include/player.h"
#include "../include/camera.h"
#include "../include/maze.h"
#include "../include/projectile.h"
#include "../include/net_utils.h"

#define SERVER_PORT 2000

typedef struct{
    ClientInfo players[MAX_PLAYERS];
} ServerInfo;

typedef struct {
    int connection_state;
    IPaddress address;
    Player *player;
    SDL_Event event;
} ClientInfo;

int main(){
    UDP_Connection conn;
    IPaddress client_ip;
    ServerInfo *data;
    ClientInfo *buffer;
    int received_len;
    int nrOfPlayers = 0;

    SDLNet_init();

    if(init_udp(&conn, SERVER_PORT) != 0){
        fprintf(stderr, "Failed to initialize UDP connection\n");
        return -1;
    }

    printf("Server is listening on port %d...\n", SERVER_PORT);

    while(1){
        received_len = receive_udp(&conn, &buffer, sizeof(buffer), &client_ip);
        if(received_len > 0){
            printf("Received %d bytes from %s:%d\n", received_len, SDLNet_ResolveIP(&client_ip),
                client_ip.port);

            if(buffer->connection_state >= 0 && buffer->connection_state <= MAX_PLAYERS){
                if(buffer->connection_state == 0 && nrOfPlayers < MAX_PLAYERS){
                    nrOfPlayers++;
                    buffer->connection_state = nrOfPlayers;
                    buffer->address = client_ip;
                }

                data->players[buffer->connection_state] = *buffer;
            }

            send_udp(&conn, &data, sizeof(data), &client_ip);
        }
    }

    close_udp(&conn);
    SDL_Quit();

    return 0;
}





struct game{
    bool isRunning;
    Player *pPlayer;
    Camera *pCamera;
    Maze *pMaze;
    Projectile *pProjectile[MAX_PROJECTILES];
}; typedef struct game Game;

int main(int argc, char** argv)
{
    (void)argc; //silence the warnings
    (void)argv;
    Game g = {0};
    if(!initiateGame(&g)) return 1;
    run(&g);
    closeGame(&g);
    return 0;
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

bool initiateGame(Game *pGame)
{
    pGame->pPlayer = createPlayer(pGame->pRenderer);

    if(!pGame->pPlayer){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return false;
    }
    pGame->pMaze = createMaze(pGame->pRenderer, pGame->tileMapTexture, pGame->tileMapSurface);
    if(!pGame->pMaze){
        printf("Maze Creation Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return false;
    }

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

    return true;
}

void closeGame(Game *pGame)
{
    // Destroy projectiles
    destroyProjectile(pGame->pProjectile);
    
    if(pGame->pPlayer) 
        destroyPlayer(pGame->pPlayer);
    if(pGame->pCamera)
        destroyCamera(pGame->pCamera);
    if(pGame->pMaze)
        destroyMaze(pGame->pMaze);
    if(pGame->pRenderer)
        SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow)
        SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}