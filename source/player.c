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
    Object_ID objectID;
    bool isAlive; // Flag to track if player is alive
}; 

Player *createPlayer(SDL_Renderer *pRenderer)
{
    Player *pPlayer = malloc(sizeof(struct player));
    SDL_Surface *pSurface = IMG_Load("resources/soldiertopdown.png");
    if(!pSurface) {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    
    pPlayer->pRenderer = pRenderer;
    pPlayer->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if(!pPlayer->pTexture) {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    
    pPlayer->x = WORLD_WIDTH / 2;
    pPlayer->y = WORLD_HEIGHT / 2;
    pPlayer->prevX = pPlayer->x;
    pPlayer->prevY = pPlayer->y;
    pPlayer->vx = 0;
    pPlayer->vy = 0;
    pPlayer->angle = 0;
    pPlayer->objectID = OBJECT_ID_PLAYER;
    pPlayer->isAlive = true; // Player starts alive
    
    // Set explicit dimensions instead of querying texture
    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
    pPlayer->playerRect.w = PLAYERWIDTH;
    pPlayer->playerRect.h = PLAYERHEIGHT;
    
    return pPlayer;
}

void drawPlayer(Player *pPlayer, Camera *pCamera)
{
    // Don't draw player if not alive
    if (!pPlayer->isAlive) return;
    
    SDL_Rect playerRect = pPlayer->playerRect;
    SDL_Rect adjustedRect = getWorldCoordinatesFromCamera(pCamera, playerRect);
    
    // Render the player with the adjusted position
    SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, NULL, &adjustedRect, pPlayer->angle + 90.0f, NULL, SDL_FLIP_NONE);
}

void updatePlayer(Player *pPlayer, float deltaTime)
{
    // Don't update movement if player is dead
    if (!pPlayer->isAlive) return;
    
    pPlayer->prevX = pPlayer->x;
    pPlayer->prevY = pPlayer->y;
    
    // Normalize diagonal movement
    if (pPlayer->vx != 0 && pPlayer->vy != 0) {
        // Moving diagonally - normalize the speed by multiplying by 0.7071 (approximately 1/sqrt(2))
        float normalizedVx = pPlayer->vx * 0.7071f;
        float normalizedVy = pPlayer->vy * 0.7071f;
        
        pPlayer->x += normalizedVx * deltaTime;
        pPlayer->y += normalizedVy * deltaTime;
    } else {
        // Moving in a cardinal direction - use full speed
        pPlayer->x += pPlayer->vx * deltaTime;
        pPlayer->y += pPlayer->vy * deltaTime;
    }
    
    // Update the player rectangle position
    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
}

// New functions for player elimination
bool isPlayerAlive(Player *pPlayer)
{
    return pPlayer->isAlive;
}

void killPlayer(Player *pPlayer)
{
    pPlayer->isAlive = false;
    pPlayer->vx = 0;
    pPlayer->vy = 0;
}

void resetPlayer(Player *pPlayer)
{
    pPlayer->isAlive = true;
}

void movePlayerLeft(Player *pPlayer) {
    if (!pPlayer->isAlive) return; // Don't move if dead
    pPlayer->vx = -PLAYERSPEED;
}

void movePlayerRight(Player *pPlayer) {
    if (!pPlayer->isAlive) return; // Don't move if dead
    pPlayer->vx = PLAYERSPEED;
}

void movePlayerUp(Player *pPlayer) {
    if (!pPlayer->isAlive) return; // Don't move if dead
    pPlayer->vy = -PLAYERSPEED;
}

void movePlayerDown(Player *pPlayer) {
    if (!pPlayer->isAlive) return; // Don't move if dead
    pPlayer->vy = PLAYERSPEED;
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

float getPlayerAngle(Player *pPlayer)
{
    return pPlayer->angle;
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