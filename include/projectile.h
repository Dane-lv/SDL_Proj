#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL.h>
#include "player.h"
#include "camera.h"

#define MAX_PROJECTILES 10
#define PROJSPEED 400

typedef struct projectile Projectile;

Projectile *createProjectile(SDL_Renderer *pRenderer);
void spawnProjectile(Projectile *pProjectile[], Player *pPlayer);
void drawProjectile(Projectile *pProjectile[], Camera *pCamera);
void updateProjectile(Projectile *pProjectile[], float deltaTime);
void destroyProjectile(Projectile *pProjectile[]);

#endif 