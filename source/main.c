#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include "../include/constants.h"
#include "../include/player.h"

struct game {
    bool isRunning;
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Player *pPlayer;
};
typedef struct game Game;

bool initiateGame(Game *pGame);
void closeGame(Game *pGame);
void run(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void updateGame(Game *pGame);
void renderGame(Game *pGame);

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

    pGame->pWindow = SDL_CreateWindow("Maze Mayham", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
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

    pGame->pPlayer = createPlayer(WINDOW_WIDTH/2,WINDOW_HEIGHT/2,pGame->pRenderer);
    if(!pGame->pPlayer){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return false;
    }
    
    return true;
}

void run(Game *pGame)
{
    pGame->isRunning = true;
    SDL_Event event;
    while(pGame->isRunning)
    {
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                pGame->isRunning = false;
            }
            else
            {
                handleInput(pGame, &event);
                updateGame(pGame);
                renderGame(pGame);
                SDL_Delay(1000 / 60 - 15);  
            }
        }
    }
}

void handleInput(Game *pGame, SDL_Event *pEvent)
{
    if(pEvent->type == SDL_KEYDOWN)
    {
        switch(pEvent->key.keysym.sym)
        {
            case SDLK_ESCAPE:
                pGame->isRunning = false;
                break;
            default:
                break;
        }
    }
}

void updateGame(Game *pGame)
{
    (void)pGame; //silence the warning
}

void renderGame(Game *pGame)
{
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);
    drawPlayer(pGame->pPlayer);

    SDL_RenderPresent(pGame->pRenderer);
}

void closeGame(Game *pGame)
{
    if(pGame->pRenderer)
        SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow)
        SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}
