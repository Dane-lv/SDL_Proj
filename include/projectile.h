// #pragma once

// ensures the file is included only once in a single compilation to prevent multiple definitions
#ifndef PROJECTILE_H
#define PROJECTILE_H

// including SDL's rendering functions for handling rendering
#include <SDL_render.h>
// including SDL's rectangle definitions for handling rectangles
#include <SDL_rect.h>

// definition of the projectile structure
typedef struct projectile Projectile;

// function declaration to create a new projectile
// takes the renderer, initial x and y positions, and initial angle (in degrees) as inputs
Projectile* create_projectile(SDL_Renderer* pRenderer, const float x_init, const float y_init,
    const float angle_degrees_init);
// function declaration to draw the projectile on the screen
// takes the SDL_Renderer for rendering and the Projectile object to be drawn
void draw_projectile(SDL_Renderer* pRenderer, Projectile* pProjectile);
// function declaration to update the projectile's state (position, velocity, etc.)
// takes a delta_time parameter to adjust the update based on time passed between frames
void update_projectile(Projectile* pProjectile, const float delta_time);
// function declaration to resolve collision between the projectile and another object
// takes the projectile to check for collision, a rectangle representing the other object,
// and an identifier for the other object to determine the collision type or handling
void resolve_projectile_collision(Projectile* pProjectile, const SDL_Rect other_object,
    const Object_ID other_object_ID);
// function declaration to normalize the angle of the projectile to ensure it stays within 0 to 360 degrees
void normalize_angle(Projectile* pProjectile);
// function declaration to destroy the projectile and free its resources
void destroy_projectile(Projectile* pProjectile);

#endif