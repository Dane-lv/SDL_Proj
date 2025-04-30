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

typedef struct{
    ClientInfo players[MAX_PLAYERS];
} ServerInfo;

typedef struct {
    int connection_state;
    IPaddress address;
    Player *player;
    SDL_Event event;
} ClientInfo;

void main(){
    UDP_Connection conn;
    if(init_udp(&conn, 0) != 0){
        // Hantera fel
    }

    //if(input == actHost_button)
    //    dataFromServer->address = SDLNet_ResolveHost("127.0.0.1",0);

    IPaddress client_addr;
    char buffer[512];

    if(receive_udp(&conn, buffer, sizeof(buffer), &client_addr)){
        printf("Mottaget: %s\n", buffer);
        send_udp(&conn, "Svar från server", 10, &client_addr);
    }
}





struct game{
    bool isRunning;
    Player *pPlayer;
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Camera *pCamera;
    Maze *pMaze;
    SDL_Texture* tileMapTexture;
    SDL_Surface* tileMapSurface;
    Projectile *pProjectile[MAX_PROJECTILES];
}; typedef struct game Game;

void renderGame(Game *pGame)
{
    // Clear with a dark background
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);
    
    // Draw background (now a solid color instead of texture)
    drawMap(pGame->pMaze, pGame->pCamera, pGame->pPlayer);
    
    // Draw player
    drawPlayer(pGame->pPlayer, pGame->pCamera);
    
    // Draw projectiles
    drawProjectile(pGame->pProjectile, pGame->pCamera);

    // Present the rendered frame
    SDL_RenderPresent(pGame->pRenderer);
}

void updatePlayerRotation(Game *pGame)
{
    // Get mouse position in screen coordinates
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
}

bool initiateGame(Game *pGame)
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
    initiateMap(pGame->pMaze);
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

void run(Game *pGame)
{
    pGame->isRunning = true;
    SDL_Event event;

    Uint32 lastTime = SDL_GetTicks(); //time of the first frame since SDl init
    Uint32 currentTime; //time when the current frame started
    float deltaTime; //move player by pixels in second instead of by frames in second

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