// #pragma once

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL_render.h>
#include <SDL_rect.h>

typedef struct projectile Projectile;

Projectile* create_projectile(SDL_Renderer* pRenderer, const float x_init, const float y_init,
    const float angle_degrees_init);
void draw_projectile(SDL_Renderer* pRenderer, Projectile* pProjectile);
void update_projectile(Projectile* pProjectile, const float delta_time);
void resolve_projectile_collision(Projectile* pProjectile, const SDL_Rect other_object);
void destroy_projectile(Projectile* pProjectile);

#endif