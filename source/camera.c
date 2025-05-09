#include <SDL.h>
#include "../include/camera.h"
#include "../include/constants.h"
#include "../include/player.h"
#include <stdlib.h>

struct camera
{
    float x, y;         // camera position
    int width, height;  // camera dimensions (viewport)
    float zoom;         // camera zoom factor
};

Camera *createCamera(int width, int height)
{
    Camera *pCamera = malloc(sizeof(struct camera));
    pCamera->x = 0;
    pCamera->y = 0;
    pCamera->width = width;
    pCamera->height = height;
    pCamera->zoom = CAMERA_ZOOM;
    return pCamera;
}

void updateCamera(Camera *pCamera, Player *pPlayer)
{
    // Get player's position
    SDL_Rect playerPos = getPlayerPosition(pPlayer);
    
    // Calculate effective viewport dimensions based on zoom
    int effectiveWidth = pCamera->width / pCamera->zoom;
    int effectiveHeight = pCamera->height / pCamera->zoom;
    
    // Always center the camera on the player, regardless of world boundaries
    pCamera->x = playerPos.x + (playerPos.w / 2) - (effectiveWidth / 2);
    pCamera->y = playerPos.y + (playerPos.h / 2) - (effectiveHeight / 2);
    
    // No boundary checks - allow camera to follow player everywhere
}

// New function for spectate mode view
void setCameraSpectateMode(Camera *pCamera, bool enabled)
{
    if (enabled) {
        // Set a zoomed out view centered on the map
        pCamera->zoom = SPECTATE_ZOOM;
        pCamera->x = WORLD_WIDTH / 2 - ((pCamera->width / pCamera->zoom) / 2);
        pCamera->y = WORLD_HEIGHT / 2 - ((pCamera->height / pCamera->zoom) / 2);
    } else {
        // Reset to normal zoom
        pCamera->zoom = CAMERA_ZOOM;
    }
}

// Set camera position directly
void setCameraPosition(Camera *pCamera, float x, float y)
{
    // Calculate effective viewport dimensions based on zoom
    int effectiveWidth = pCamera->width / pCamera->zoom;
    int effectiveHeight = pCamera->height / pCamera->zoom;
    
    // Center the camera on the given coordinates
    pCamera->x = x - (effectiveWidth / 2);
    pCamera->y = y - (effectiveHeight / 2);
}

SDL_Rect getWorldCoordinatesFromCamera(Camera *pCamera, SDL_Rect entityRect)
{
    SDL_Rect cameraAdjustedRect = entityRect;
    
    // Calculate screen coordinates by subtracting camera position and applying zoom
    cameraAdjustedRect.x = (int)((entityRect.x - pCamera->x) * pCamera->zoom);
    cameraAdjustedRect.y = (int)((entityRect.y - pCamera->y) * pCamera->zoom);
    cameraAdjustedRect.w = (int)(entityRect.w * pCamera->zoom);
    cameraAdjustedRect.h = (int)(entityRect.h * pCamera->zoom);
    
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