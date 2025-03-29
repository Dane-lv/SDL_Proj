#include <SDL.h>
#include <SDL_image.h>
#include "../include/player.h"
#include "../include/constants.h"

struct player
{
    float x, y; //player movement math
    float vy, vx;
    int angle;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect playerRect; //rect.x rect.y for rendering after movement math
}; 

Player *createPlayer(int x, int y, SDL_Renderer *pRenderer)
{
    Player *pPlayer = malloc(sizeof(struct player));
    pPlayer->x = x;
    pPlayer->y = y;
    pPlayer->vy = pPlayer->vx = 0;
    pPlayer->angle = 0;
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

void updatePlayer(Player *pPlayer, float deltaTime)
{
    pPlayer->x += pPlayer->vx * deltaTime;
    pPlayer->y += pPlayer->vy * deltaTime;
    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
} 

void movePlayerLeft(Player *pPlayer) {
    pPlayer->vx = -PLAYERSPEED;
    
}

void movePlayerRight(Player *pPlayer) {
    pPlayer->vx = +PLAYERSPEED;

}

void movePlayerUp(Player *pPlayer) {
    pPlayer->vy = -PLAYERSPEED;
    
}

void movePlayerDown(Player *pPlayer) {
    pPlayer->vy = +PLAYERSPEED;
    
} 

void stopMovementVY(Player *pPlayer)
{
    pPlayer->vy = 0;   
}

void stopMovementVX(Player *pPlayer)
{
    pPlayer->vx = 0;   
}


void destroyPlayer(Player *pPlayer)
{
    free(pPlayer);
}