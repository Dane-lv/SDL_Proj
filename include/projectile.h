#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL.h>
#include "player.h"
#include "camera.h"
#include "constants.h"
#include "maze.h"

typedef struct projectile Projectile;

Projectile *createProjectile(SDL_Renderer *pRenderer);
void spawnProjectile(Projectile *pProjectile[], Player *pPlayer);
void drawProjectile(Projectile *pProjectile[], Camera *pCamera);
void updateProjectile(Projectile *pProjectile[], float deltaTime);
void updateProjectileWithWallCollision(Projectile *pProjectile[], Maze *pMaze, float deltaTime);
void destroyProjectile(Projectile *pProjectile[]);
SDL_Rect getProjectileRect(Projectile *pProjectile);
bool isProjectileActive(Projectile *pProjectile);
float getProjectileAge(Projectile *pProjectile);

#endif 