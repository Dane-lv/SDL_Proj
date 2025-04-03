#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL.h>

typedef struct projectile Projectile;

Projectile *createProjectile(float x_init, float y_init, float angle_init, SDL_Renderer *pRenderer);
void drawProjectile(SDL_Renderer *pRenderer, Projectile *pProjectile);
void updateProjectile(Projectile *pProjectile, float delta_time);
void resolveProjectileCollision(Projectile *pProjectile, SDL_Rect other_object);
void destroyProjectile(Projectile *pProjectile);

#endif