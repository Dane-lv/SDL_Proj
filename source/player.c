#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "../include/player.h"
#include "../include/constants.h"

struct player
{
    float x, y; //player movement math
    float prevX, prevY; // Previous position for collision resolution
    float vy, vx;
    float angle;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect playerRect; //rect.x rect.y for rendering after movement math calc
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
    SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, NULL, &(pPlayer->playerRect), pPlayer->angle + 90.0f, NULL, SDL_FLIP_NONE);
}

void updatePlayer(Player *pPlayer, float deltaTime)
{
    // spara föredetta position 
    pPlayer->prevX = pPlayer->x;
    pPlayer->prevY = pPlayer->y;
    
    // Uppdatera tid
    pPlayer->x += pPlayer->vx * deltaTime;
    pPlayer->y += pPlayer->vy * deltaTime;
    

    if(pPlayer->x < 0) pPlayer->x = 0;
    if(pPlayer->y < 0) pPlayer->y = 0;
    if(pPlayer->x > (WINDOW_WIDTH - pPlayer->playerRect.w)) pPlayer->x = WINDOW_WIDTH - pPlayer->playerRect.w;
    if(pPlayer->y > (WINDOW_HEIGHT - pPlayer->playerRect.h)) pPlayer->y = WINDOW_HEIGHT - pPlayer->playerRect.h;

    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;

    //current mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    //center of the player
    float playerCenterX = pPlayer->playerRect.x + pPlayer->playerRect.w / 2.0f;
    float playerCenterY = pPlayer->playerRect.y + pPlayer->playerRect.h / 2.0f;
    
    // angle between player and mouse cursor
    float deltaX = mouseX - playerCenterX;
    float deltaY = mouseY - playerCenterY;
    //få vinkel (tan) i radianer
    float radians = atan2f(deltaY, deltaX);
    //radianer till grader
    pPlayer->angle = radians * 180.0f / M_PI;
    
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

SDL_Rect getPlayerRect(Player *pPlayer)
{
    return pPlayer->playerRect;
}

void setPlayerPosition(Player *pPlayer, float x, float y)
{
    pPlayer->x = x;
    pPlayer->y = y;
    pPlayer->playerRect.x = (int)x;
    pPlayer->playerRect.y = (int)y;
}

void destroyPlayer(Player *pPlayer)
{
    free(pPlayer);
}

void revertToPreviousPosition(Player *pPlayer)
{
    pPlayer->x = pPlayer->prevX;
    pPlayer->y = pPlayer->prevY;
    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
}