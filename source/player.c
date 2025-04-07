#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "../include/player.h"
#include "../include/constants.h"
#include "../include/camera.h"

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

Player *createPlayer(SDL_Renderer *pRenderer)
{
    Player *pPlayer = malloc(sizeof(struct player));
    // Spawn player in a safe location (center of the main area)
    pPlayer->x = 400;
    pPlayer->y = 300;
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

void drawPlayer(Player *pPlayer, Camera *pCamera)
{
    // Create a copy of player rect for camera adjustments
    SDL_Rect playerRect = pPlayer->playerRect;
    
    // Apply camera transformation
    SDL_Rect adjustedRect = getWorldCoordinatesFromCamera(pCamera, playerRect);
    
    // Render the player with the adjusted position
    SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, NULL, &adjustedRect, pPlayer->angle + 90.0f, NULL, SDL_FLIP_NONE);
}

void updatePlayer(Player *pPlayer, float deltaTime)
{
    // Save previous position for collision resolution
    pPlayer->prevX = pPlayer->x;
    pPlayer->prevY = pPlayer->y;
    
    // Update position based on velocity and time
    pPlayer->x += pPlayer->vx * deltaTime;
    pPlayer->y += pPlayer->vy * deltaTime;
    
    // Update the player's rectangle for rendering and collision
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

SDL_Rect getPlayerPosition(Player *pPlayer)
{
    return pPlayer->playerRect;
}

SDL_Rect getPlayerRect(Player *pPlayer)
{
    return pPlayer->playerRect;
}

SDL_Texture *getPlayerTexture(Player *pPlayer)
{
    return pPlayer->pTexture;
}

void setPlayerPosition(Player *pPlayer, float x, float y)
{
    pPlayer->x = x;
    pPlayer->y = y;
    pPlayer->playerRect.x = (int)x;
    pPlayer->playerRect.y = (int)y;
}

void setPlayerAngle(Player *pPlayer, float angle)
{
    pPlayer->angle = angle;
}

void destroyPlayer(Player *pPlayer)
{
    if (pPlayer->pTexture) {
        SDL_DestroyTexture(pPlayer->pTexture);
    }
    free(pPlayer);
}

void revertToPreviousPosition(Player *pPlayer)
{
    pPlayer->x = pPlayer->prevX;
    pPlayer->y = pPlayer->prevY;
    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
}