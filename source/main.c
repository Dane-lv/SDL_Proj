#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include "../include/constants.h"
#include "../include/player.h"
#include "../include/camera.h"

// Size of each checkerboard square FOR VISUALISATION
#define CHECKER_SIZE 64

struct game {
    bool isRunning;
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Player *pPlayer;
    Camera *pCamera;
};
typedef struct game Game;

bool initiateGame(Game *pGame);
void closeGame(Game *pGame);
void run(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void updateGame(Game *pGame, float deltaTime);
void renderGame(Game *pGame);
void drawCheckerboard(Game *pGame);

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

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, 0);
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
    
    pGame->pCamera = createCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!pGame->pCamera){
        printf("Error: Failed to create camera\n");
        closeGame(pGame);
        return false;
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
                stopMovementVY(pGame->pPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_D:
                stopMovementVX(pGame->pPlayer);
                break;
            default:
                break;
        }
    }
}

void updateGame(Game *pGame, float deltaTime)
{
    updatePlayer(pGame->pPlayer, deltaTime);
    updateCamera(pGame->pCamera, pGame->pPlayer);
}

void drawCheckerboard(Game *pGame) //FOR VISUALISATION
{
    // Get camera position
    int camX = getCameraX(pGame->pCamera);
    int camY = getCameraY(pGame->pCamera);
    
    // Calculate the starting grid position (which checker we're seeing at top-left)
    int startX = camX / CHECKER_SIZE;
    int startY = camY / CHECKER_SIZE;
    
    // Calculate offset to ensure smooth scrolling
    int offsetX = -(camX % CHECKER_SIZE);
    int offsetY = -(camY % CHECKER_SIZE);
    
    // Draw the checkerboard
    for (int y = 0; y <= WINDOW_HEIGHT / CHECKER_SIZE + 1; y++) {
        for (int x = 0; x <= WINDOW_WIDTH / CHECKER_SIZE + 1; x++) {
            SDL_Rect rect = {
                offsetX + x * CHECKER_SIZE,
                offsetY + y * CHECKER_SIZE,
                CHECKER_SIZE,
                CHECKER_SIZE
            };
            
            // Determine if this square should be white or black
            if ((startX + x + startY + y) % 2 == 0) {
                SDL_SetRenderDrawColor(pGame->pRenderer, 200, 200, 200, 255); // Light gray
            } else {
                SDL_SetRenderDrawColor(pGame->pRenderer, 50, 50, 50, 255); // Dark gray
            }
            
            SDL_RenderFillRect(pGame->pRenderer, &rect);
        }
    }
}

void renderGame(Game *pGame)
{
    // Clear with a dark background
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);
    
    // Draw the checkerboard pattern FOR VISUALISATION
    drawCheckerboard(pGame);
    
    // Get player's original rectangle
    SDL_Rect playerPos = getPlayerPosition(pGame->pPlayer);
    
    // Adjust for camera
    SDL_Rect adjustedPos = getWorldCoordinatesFromCamera(pGame->pCamera, playerPos);
    
    // Draw player with adjusted coordinates
    SDL_RenderCopy(pGame->pRenderer, getPlayerTexture(pGame->pPlayer), NULL, &adjustedPos);
    
    SDL_RenderPresent(pGame->pRenderer);
}

void closeGame(Game *pGame)
{
    if(pGame->pPlayer) 
        destroyPlayer(pGame->pPlayer);
    if(pGame->pCamera)
        destroyCamera(pGame->pCamera);
    if(pGame->pRenderer)
        SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow)
        SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}
