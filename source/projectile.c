#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include "../include/projectile.h"
#include "../include/constants.h"
#include "../include/player.h"
#include "../include/camera.h"

struct projectile
{
    float x, y;
    float vx, vy;
    bool isActive;
    float projDuration;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect projRect;
};


Projectile *createProjectile(SDL_Renderer *pRenderer)
{
    Projectile *pProjectile = malloc(sizeof(struct projectile));
    SDL_Surface *pSurface = IMG_Load("resources/projectile.png");
    if(!pSurface) {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    pProjectile->pRenderer = pRenderer;
    pProjectile->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    
    if(!pProjectile->pTexture) {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_QueryTexture(pProjectile->pTexture, NULL, NULL, &(pProjectile->projRect.w), &(pProjectile->projRect.h));
    pProjectile->projRect.w /= 2;
    pProjectile->projRect.h /= 2;
    pProjectile->isActive = false;
    
    return pProjectile;
}

void spawnProjectile(Projectile *pProjectile[], Player *pPlayer)
{
    for(int i = 0; i < MAX_PROJECTILES; i++)
    {
        if(pProjectile[i]->isActive == false)
        {
            pProjectile[i]->isActive = true;
            
            SDL_Rect playerRect = getPlayerRect(pPlayer);
            float playerCenterX = playerRect.x + playerRect.w / 2.0f;
            float playerCenterY = playerRect.y + playerRect.h / 2.0f;
            
            
            pProjectile[i]->x = playerCenterX;
            pProjectile[i]->y = playerCenterY;
            
            
            pProjectile[i]->projRect.x = (int)pProjectile[i]->x - pProjectile[i]->projRect.w / 2;
            pProjectile[i]->projRect.y = (int)pProjectile[i]->y - pProjectile[i]->projRect.h / 2;
            
            
            float angle = getPlayerAngle(pPlayer);
            float radians = (angle) * M_PI / 180.0f; 
            
            
            pProjectile[i]->vx = cosf(radians) * PROJSPEED;
            pProjectile[i]->vy = sinf(radians) * PROJSPEED;
            
            
            pProjectile[i]->projDuration = 3.0f;  // 3 seconds duration
            
            return; // Exit after spawning one projectile
        }
    }
}

void drawProjectile(Projectile *pProjectile[], Camera *pCamera)
{
    for(int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (pProjectile[i]->isActive) 
        {
            
            pProjectile[i]->projRect.x = (int)pProjectile[i]->x - pProjectile[i]->projRect.w / 2;
            pProjectile[i]->projRect.y = (int)pProjectile[i]->y - pProjectile[i]->projRect.h / 2;

          
            SDL_Rect adjustedRect = getWorldCoordinatesFromCamera(pCamera, pProjectile[i]->projRect);

            // Render with camera-adjusted coordinates
            SDL_RenderCopy(
                pProjectile[i]->pRenderer,
                pProjectile[i]->pTexture,
                NULL,
                &adjustedRect
            );
        }
    }
}

void projBounce(Projectile *pProjectile)
{
    int halfW = pProjectile->projRect.w / 2;
    int halfH = pProjectile->projRect.h / 2;
    
    
    if (pProjectile->x - halfW <= 0) 
    {
        pProjectile->x = halfW; 
        pProjectile->vx = -pProjectile->vx; 
    }
    else if (pProjectile->x + halfW >= WORLD_WIDTH) 
    {
        pProjectile->x = WORLD_WIDTH - halfW; 
        pProjectile->vx = -pProjectile->vx; 
    }
    if (pProjectile->y - halfH <= 0) {
        pProjectile->y = halfH; 
        pProjectile->vy = -pProjectile->vy;
    }
    else if (pProjectile->y + halfH >= WORLD_HEIGHT) {
        pProjectile->y = WORLD_HEIGHT - halfH; 
        pProjectile->vy = -pProjectile->vy;
    }
}

void updateProjectile(Projectile *pProjectile[], float deltaTime)
{
    for(int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (pProjectile[i]->isActive) 
        {
            pProjectile[i]->projDuration -= deltaTime;
            if (pProjectile[i]->projDuration > 0) 
            {
                pProjectile[i]->x += pProjectile[i]->vx * deltaTime;
                pProjectile[i]->y += pProjectile[i]->vy * deltaTime;
                projBounce(pProjectile[i]);
            }
            else 
            {
                pProjectile[i]->isActive = false;
            }
        }
    }
}


void destroyProjectile(Projectile *pProjectile[])
{
    for(int i = 0; i < MAX_PROJECTILES; i++)
    {
        if(pProjectile[i])
        {
            if (pProjectile[i]->pTexture) 
            {
                SDL_DestroyTexture(pProjectile[i]->pTexture);
            }
            free(pProjectile[i]);
        }
    }
} 