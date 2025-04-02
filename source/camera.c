#include <SDL.h>
#include "../include/camera.h"
#include "../include/constants.h"
#include "../include/player.h"
#include <stdlib.h>

struct camera
{
    float x, y;         // camera position
    int width, height;  // camera dimensions (viewport)
};

Camera *createCamera(int width, int height)
{
    Camera *pCamera = malloc(sizeof(struct camera));
    pCamera->x = 0;
    pCamera->y = 0;
    pCamera->width = width;
    pCamera->height = height;
    return pCamera;
}

void updateCamera(Camera *pCamera, Player *pPlayer)
{
    // Get player's position
    SDL_Rect playerPos = getPlayerPosition(pPlayer);
    
    // Center the camera on the player
    pCamera->x = playerPos.x + (playerPos.w / 2) - (pCamera->width / 2);
    pCamera->y = playerPos.y + (playerPos.h / 2) - (pCamera->height / 2);
    
    // Ensure camera doesn't show beyond the world boundaries
    if (pCamera->x < 0) pCamera->x = 0;
    if (pCamera->y < 0) pCamera->y = 0;
    if (pCamera->x > WORLD_WIDTH - pCamera->width) pCamera->x = WORLD_WIDTH - pCamera->width;
    if (pCamera->y > WORLD_HEIGHT - pCamera->height) pCamera->y = WORLD_HEIGHT - pCamera->height;
}

SDL_Rect getWorldCoordinatesFromCamera(Camera *pCamera, SDL_Rect entityRect)
{
    SDL_Rect cameraAdjustedRect = entityRect;
    
    // Calculate screen coordinates by subtracting camera position
    cameraAdjustedRect.x = entityRect.x - (int)pCamera->x;
    cameraAdjustedRect.y = entityRect.y - (int)pCamera->y;
    
    return cameraAdjustedRect;
}

int getCameraX(Camera *pCamera)
{
    return (int)pCamera->x;
}

int getCameraY(Camera *pCamera)
{
    return (int)pCamera->y;
}

void destroyCamera(Camera *pCamera)
{
    free(pCamera);
} 