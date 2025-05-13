#ifndef CAMERA_H
#define CAMERA_H

#include <SDL.h>
#include <stdbool.h>

// Forward declaration to avoid circular include
typedef struct player Player;
typedef struct camera Camera;

Camera *createCamera(int width, int height);
void updateCamera(Camera *pCamera, Player *pPlayer);
void destroyCamera(Camera *pCamera);
SDL_Rect getWorldCoordinatesFromCamera(Camera *pCamera, SDL_Rect entityRect);
int getCameraX(Camera *pCamera);
int getCameraY(Camera *pCamera);

// New function
void setCameraSpectateMode(Camera *pCamera, bool enabled);

// New functions
void setCameraPosition(Camera *pCamera, float x, float y);

#endif 