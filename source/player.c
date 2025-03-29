#include <SDL.h>
#include <SDL_image.h>
#include "../include/player.h"
#include "../include/constants.h"

struct player
{
    float x;
    float y;
    float playerSpeed;
    int angle;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect playerRect;
}; 

Player *createPlayer(int x, int y, SDL_Renderer *pRenderer)
{
    Player *pPlayer = malloc(sizeof(struct player));
    pPlayer->x = x;
    pPlayer->y = y;
    pPlayer->playerSpeed = PLAYERSPEED;
    SDL_Surface *pSurface = IMG_Load("resources/soldiertopdown.png");
    if(!pSurface){
        printf("Error: %s\n",SDL_GetError());
        return NULL;
    }
    pPlayer->pRenderer = pRenderer;
    pPlayer->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if(!pPlayer->pTexture){
        printf("Error: %s\n",SDL_GetError());
        return NULL;
    }
    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
    pPlayer->playerRect.w = PLAYERWIDTH;
    pPlayer->playerRect.h = PLAYERHEIGHT;
    return pPlayer;
}

void drawPlayer(Player *pPlayer)
{
    SDL_RenderCopy(pPlayer->pRenderer,pPlayer->pTexture,NULL,&(pPlayer->playerRect));
}