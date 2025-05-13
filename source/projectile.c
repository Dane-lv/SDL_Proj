#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdbool.h>
#include "../include/projectile.h"
#include "../include/constants.h"
#include "../include/player.h"
#include "../include/camera.h"
#include "../include/maze.h"

struct projectile
{
    float x, y;
    float vx, vy;
    bool isActive;
    float projDuration;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect projRect;
    Object_ID objectID;
    Player* owner;  // Player who fired this projectile
    float distanceTraveled;  // Track total distance traveled
    bool hasBounced;         // Flag to track if projectile has bounced
    float minOwnerCollisionDistance; // Minimum distance before it can hit owner
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
    pProjectile->objectID = OBJECT_ID_PROJECTILE;
    pProjectile->owner = NULL; // Initialize owner as NULL
    pProjectile->distanceTraveled = 0.0f;
    pProjectile->hasBounced = false;
    pProjectile->minOwnerCollisionDistance = PLAYERWIDTH * 3.0f; // Safe distance (3x player width)
    
    return pProjectile;
}

int spawnProjectile(Projectile *pProjectile[], Player *pPlayer)
{
    for(int i = 0; i < MAX_PROJECTILES; i++)
    {
        if(pProjectile[i]->isActive == false)
        {
            pProjectile[i]->isActive = true;
            pProjectile[i]->owner = pPlayer; // Set the owner to the player who fired
            
            SDL_Rect playerRect = getPlayerRect(pPlayer);
            float playerCenterX = playerRect.x + playerRect.w / 2.0f;
            float playerCenterY = playerRect.y + playerRect.h / 2.0f;
            
            // Set initial position to be just outside the player
            float angle = getPlayerAngle(pPlayer);
            float radians = (angle) * M_PI / 180.0f; 
            
            // Use minimal offset to prevent immediate collision
            float offsetDistance = 5.0f;  // Minimal offset, just enough to prevent immediate collision
            playerCenterX += cosf(radians) * offsetDistance;
            playerCenterY += sinf(radians) * offsetDistance;
            
            pProjectile[i]->x = playerCenterX;
            pProjectile[i]->y = playerCenterY;
            
            pProjectile[i]->projRect.x = (int)pProjectile[i]->x - pProjectile[i]->projRect.w / 2;
            pProjectile[i]->projRect.y = (int)pProjectile[i]->y - pProjectile[i]->projRect.h / 2;
            
            pProjectile[i]->vx = cosf(radians) * PROJSPEED;
            pProjectile[i]->vy = sinf(radians) * PROJSPEED;
            
            pProjectile[i]->projDuration = 3.0f;  // 3 seconds duration
            pProjectile[i]->distanceTraveled = 0.0f;
            pProjectile[i]->hasBounced = false;
            
            return i; // Return the projectile index
        }
    }
    return -1; // No available projectile
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

// Bounce off world boundaries
void projBounceWorld(Projectile *pProjectile)
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

// Check and handle wall collisions
bool projBounceWall(Projectile *pProjectile, Maze *pMaze)
{
    // Create temp variables for potential next position
    float nextX = pProjectile->x + pProjectile->vx * 0.01f; // Small movement to detect collision before it happens
    float nextY = pProjectile->y + pProjectile->vy * 0.01f;
    
    // Create a temporary rect for collision testing
    SDL_Rect tempRect = pProjectile->projRect;
    tempRect.x = (int)nextX - tempRect.w / 2;
    tempRect.y = (int)nextY - tempRect.h / 2;
    
    // Check for collision with any wall
    if (checkCollision(pMaze, tempRect)) {
        // Determine collision axis (X or Y or both)
        SDL_Rect xOnlyRect = pProjectile->projRect;
        xOnlyRect.x = (int)nextX - xOnlyRect.w / 2;
        xOnlyRect.y = pProjectile->projRect.y;
        
        SDL_Rect yOnlyRect = pProjectile->projRect;
        yOnlyRect.x = pProjectile->projRect.x;
        yOnlyRect.y = (int)nextY - yOnlyRect.h / 2;
        
        bool xCollision = checkCollision(pMaze, xOnlyRect);
        bool yCollision = checkCollision(pMaze, yOnlyRect);
        
        if (xCollision) {
            pProjectile->vx = -pProjectile->vx;
        }
        
        if (yCollision) {
            pProjectile->vy = -pProjectile->vy;
        }
        
        // If no specific axis collision detected but there is a collision, reverse both
        if (!xCollision && !yCollision) {
            pProjectile->vx = -pProjectile->vx;
            pProjectile->vy = -pProjectile->vy;
        }
        
        return true;
    }
    
    return false;
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
                // Store old position to calculate distance moved
                float oldX = pProjectile[i]->x;
                float oldY = pProjectile[i]->y;
                
                // Update position
                pProjectile[i]->x += pProjectile[i]->vx * deltaTime;
                pProjectile[i]->y += pProjectile[i]->vy * deltaTime;
                
                // Calculate distance traveled this frame
                float dx = pProjectile[i]->x - oldX;
                float dy = pProjectile[i]->y - oldY;
                float distanceThisFrame = sqrtf(dx*dx + dy*dy);
                pProjectile[i]->distanceTraveled += distanceThisFrame;
                
                projBounceWorld(pProjectile[i]);
            }
            else 
            {
                pProjectile[i]->isActive = false;
            }
        }
    }
}

// Update with wall collision detection
void updateProjectileWithWallCollision(Projectile *pProjectile[], Maze *pMaze, float deltaTime)
{
    for(int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (pProjectile[i]->isActive) 
        {
            pProjectile[i]->projDuration -= deltaTime;
            if (pProjectile[i]->projDuration > 0) 
            {
                // Store old position to calculate distance moved
                float oldX = pProjectile[i]->x;
                float oldY = pProjectile[i]->y;
                
                // Update position
                pProjectile[i]->x += pProjectile[i]->vx * deltaTime;
                pProjectile[i]->y += pProjectile[i]->vy * deltaTime;
                
                // Calculate distance traveled this frame
                float dx = pProjectile[i]->x - oldX;
                float dy = pProjectile[i]->y - oldY;
                float distanceThisFrame = sqrtf(dx*dx + dy*dy);
                pProjectile[i]->distanceTraveled += distanceThisFrame;
                
                // Check for world boundary collision
                projBounceWorld(pProjectile[i]);
                
                // Check for wall collision
                if (projBounceWall(pProjectile[i], pMaze)) {
                    pProjectile[i]->hasBounced = true;
                }
            }
            else 
            {
                pProjectile[i]->isActive = false;
            }
        }
    }
}

SDL_Rect getProjectileRect(Projectile *pProjectile)
{
    return pProjectile->projRect;
}

bool isProjectileActive(Projectile *pProjectile)
{
    return pProjectile->isActive;
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

// New function to get the owner of the projectile
Player* getProjectileOwner(Projectile *pProjectile) {
    return pProjectile->owner;
}

// New function for projectile-player collision detection
bool checkProjectilePlayerCollision(Projectile *pProjectile, Player *pPlayer) {
    if (!pProjectile->isActive || !isPlayerAlive(pPlayer)) {
        return false;
    }
    
    // Allow self-collision only if the projectile has traveled enough distance or has bounced
    if (pProjectile->owner == pPlayer) {
        if (!pProjectile->hasBounced && pProjectile->distanceTraveled < pProjectile->minOwnerCollisionDistance) {
            return false;
        }
    }
    
    SDL_Rect projRect = pProjectile->projRect;
    SDL_Rect playerRect = getPlayerRect(pPlayer);
    
    // Using SDL's built-in rectangle collision detection
    return SDL_HasIntersection(&projRect, &playerRect);
}

void deactivateProjectile(Projectile *pProjectile) {
    pProjectile->isActive = false;
}

// New functions for network synchronization
void setProjectileActive(Projectile *pProjectile, bool active) {
    pProjectile->isActive = active;
    if (active) {
        // network-spawns need a fresh slate too
        pProjectile->distanceTraveled = 0.0f;
        pProjectile->hasBounced = false;
    }
}

void setProjectileOwner(Projectile *pProjectile, Player *owner) {
    pProjectile->owner = owner;
}

void setProjectilePosition(Projectile *pProjectile, float x, float y) {
    pProjectile->x = x;
    pProjectile->y = y;
    pProjectile->projRect.x = (int)x - pProjectile->projRect.w / 2;
    pProjectile->projRect.y = (int)y - pProjectile->projRect.h / 2;
}

void setProjectileVelocity(Projectile *pProjectile, float vx, float vy) {
    pProjectile->vx = vx;
    pProjectile->vy = vy;
}

void setProjectileDuration(Projectile *pProjectile, float duration) {
    pProjectile->projDuration = duration;
}

void setProjectileHasBounced(Projectile *pProjectile, bool hasBounced) {
    pProjectile->hasBounced = hasBounced;
}

float getProjectileDistanceTraveled(Projectile *pProjectile) {
    return pProjectile->distanceTraveled;
}

bool getProjectileHasBounced(Projectile *pProjectile) {
    return pProjectile->hasBounced;
}

// Find an available projectile slot
int findAvailableProjectileIndex(Projectile *pProjectiles[]) {
    for(int i = 0; i < MAX_PROJECTILES; i++) {
        if(pProjectiles[i] && !pProjectiles[i]->isActive) {
            return i;
        }
    }
    return -1; // No available projectile
} 