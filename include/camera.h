#ifndef CAMERA_H
#define CAMERA_H

#include <SDL.h>

// Forward declaration to avoid circular include
typedef struct player Player;
typedef struct camera Camera;

Camera *createCamera(int width, int height);
void updateCamera(Camera *pCamera, Player *pPlayer);
void destroyCamera(Camera *pCamera);
SDL_Rect getWorldCoordinatesFromCamera(Camera *pCamera, SDL_Rect entityRect);
int getCameraX(Camera *pCamera);
int getCameraY(Camera *pCamera);

#endif 