#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL.h>
#include "player.h"
#include "camera.h"
#include "constants.h"
#include "maze.h"

typedef struct projectile Projectile;

Projectile *createProjectile(SDL_Renderer *pRenderer);
int spawnProjectile(Projectile *pProjectile[], Player *pPlayer);
void drawProjectile(Projectile *pProjectile[], Camera *pCamera);
void updateProjectile(Projectile *pProjectile[], float deltaTime);
void updateProjectileWithWallCollision(Projectile *pProjectile[], Maze *pMaze, float deltaTime);
void destroyProjectile(Projectile *pProjectile[]);
SDL_Rect getProjectileRect(Projectile *pProjectile);
bool isProjectileActive(Projectile *pProjectile);

// New function for projectile-player collision detection
bool checkProjectilePlayerCollision(Projectile *pProjectile, Player *pPlayer);
void deactivateProjectile(Projectile *pProjectile);

// Get the owner of the projectile
Player* getProjectileOwner(Projectile *pProjectile);

// New functions for network synchronization
void setProjectileActive(Projectile *pProjectile, bool active);
void setProjectileOwner(Projectile *pProjectile, Player *owner);
void setProjectilePosition(Projectile *pProjectile, float x, float y);
void setProjectileVelocity(Projectile *pProjectile, float vx, float vy);
void setProjectileDuration(Projectile *pProjectile, float duration);
void setProjectileHasBounced(Projectile *pProjectile, bool hasBounced);
float getProjectileDistanceTraveled(Projectile *pProjectile);
bool getProjectileHasBounced(Projectile *pProjectile);
int findAvailableProjectileIndex(Projectile *pProjectiles[]);

#endif 