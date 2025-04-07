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
    
    // Calculate effective viewport dimensions based on zoom
    int effectiveWidth = pCamera->width / CAMERA_ZOOM;
    int effectiveHeight = pCamera->height / CAMERA_ZOOM;
    
    // Always center the camera on the player, regardless of world boundaries
    pCamera->x = playerPos.x + (playerPos.w / 2) - (effectiveWidth / 2);
    pCamera->y = playerPos.y + (playerPos.h / 2) - (effectiveHeight / 2);
    
    // No boundary checks - allow camera to follow player everywhere
}

SDL_Rect getWorldCoordinatesFromCamera(Camera *pCamera, SDL_Rect entityRect)
{
    SDL_Rect cameraAdjustedRect = entityRect;
    
    // Calculate screen coordinates by subtracting camera position and applying zoom
    cameraAdjustedRect.x = (int)((entityRect.x - pCamera->x) * CAMERA_ZOOM);
    cameraAdjustedRect.y = (int)((entityRect.y - pCamera->y) * CAMERA_ZOOM);
    cameraAdjustedRect.w = (int)(entityRect.w * CAMERA_ZOOM);
    cameraAdjustedRect.h = (int)(entityRect.h * CAMERA_ZOOM);
    
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