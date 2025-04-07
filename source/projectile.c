#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <floats.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../include/constants.h"
#include "../include/projectile.h"
#include "../include/rectangle_obb_corners.h"

struct projectile{
    float pos_x, pos_y, angle_degrees, velocity;
    SDL_Rect bounding_box;
    SDL_Texture* pTexture;
}; typedef struct projectile Projectile;

Projectile* create_projectile(SDL_Renderer* pRenderer, const float x_init, const float y_init,
        const float angle_degrees_init){
    Projectile* pProjectile = malloc(sizeof(Projectile));
    if(!pProjectile){
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }

    pProjectile->pos_x = x_init;
    pProjectile->pos_y = y_init;
    pProjectile->angle_degrees = angle_degrees_init;
    normalize_angle(pProjectile);

    pProjectile->bounding_box.w = PROJECTILE_WIDTH;
    pProjectile->bounding_box.h = PROJECTILE_HEIGHT;

    pProjectile->velocity = PROJECTILE_VELOCITY_INIT;

    SDL_Surface* pSurface = SDL_CreateRGBSurface(0, 100, 100, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);//IMG_Load("resources/xxx.png");
    if(!pSurface){
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_FillRect(pSurface, NULL, SDL_MapRGB(pSurface->format, 255, 255, 255));

    (pProjectile->pTexture) = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if(!(pProjectile->pTexture)){
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    
    return pProjectile;
}

void draw_projectile(SDL_Renderer* pRenderer, Projectile* pProjectile){
    SDL_RenderCopyEx(pRenderer, pProjectile->pTexture, NULL, &(pProjectile->bounding_box),
        pProjectile->angle_degrees, NULL, SDL_FLIP_NONE);
}

void update_projectile(Projectile* pProjectile, const float delta_time){
    float angle_radians = (pProjectile->angle_degrees * M_PI) / 180.0f; // Omvandla till radianer
    float dx = pProjectile->velocity * cos(angle_radians) * delta_time;    // Beräkna hastighetskomponenter
    float dy = pProjectile->velocity * sin(angle_radians) * delta_time;

    pProjectile->pos_x += dx;           // Uppdatera position
    pProjectile->pos_y += dy;

    pProjectile->bounding_box.x = pProjectile->pos_x;
    pProjectile->bounding_box.y = pProjectile->pos_y;

    if(pProjectile->bounding_box.x < 0)
        pProjectile->bounding_box.x = 0;
    else if(pProjectile->bounding_box.x > (WINDOW_WIDTH - PROJECTILE_WIDTH))
        pProjectile->bounding_box.x = WINDOW_WIDTH - PROJECTILE_WIDTH;

    if(pProjectile->bounding_box.y < 0)
        pProjectile->bounding_box.y = 0;
    else if(pProjectile->bounding_box.y > (WINDOW_HEIGHT - PROJECTILE_HEIGHT))
        pProjectile->bounding_box.y = WINDOW_HEIGHT - PROJECTILE_HEIGHT;
    
    if((int) pProjectile->pos_x != pProjectile->bounding_box.x)
        pProjectile->pos_x = pProjectile->bounding_box.x;
    if((int) pProjectile->pos_y != pProjectile->bounding_box.y)
        pProjectile->pos_y = pProjectile->bounding_box.y;
}

void resolve_projectile_collision(Projectile* pProjectile, const SDL_Rect other_object){
    SDL_FPoint corners[4];
    corners_rotate(pProjectile->bounding_box, pProjectile->angle_degrees, corners);

    for(int i = 0; i < 4; i++){
        if(corners_overlap(corners[i], other_object))
            return;
    }

    float angle_degrees = pProjectile->angle_degrees;
    if((int) angle_degrees % 90 == 0){
        if(angle_degrees == PROJECTILE_ANGLE_RIGHT)
            pProjectile->angle_degrees = PROJECTILE_ANGLE_LEFT;
        else if(angle_degrees == PROJECTILE_ANGLE_DOWN)
            pProjectile->angle_degrees = PROJECTILE_ANGLE_UP;
        else if(angle_degrees == PROJECTILE_ANGLE_LEFT)
            pProjectile->angle_degrees = PROJECTILE_ANGLE_RIGHT;
        else if(angle_degrees == PROJECTILE_ANGLE_UP)
            pProjectile->angle_degrees = PROJECTILE_ANGLE_DOWN;
        return;
    }

    float x_min_distance = FLT_MAX / 2.0f;
    float y_min_distance = FLT_MAX / 2.0f;
    for(int i = 0; i < 4; i++){
        if(angle_degrees > PROJECTILE_ANGLE_RIGHT && angle_degrees < PROJECTILE_ANGLE_DOWN){
            if(fabsf(corners[i].x - other_object.x) < x_min_distance)
                x_min_distance = fabsf(corners[i].x - other_object.x);
            if(fabsf(corners[i].y - other_object.y) < y_min_distance)
                y_min_distance = fabsf(corners[i].y - other_object.y);
        }
        else if(angle_degrees > PROJECTILE_ANGLE_DOWN && angle_degrees < PROJECTILE_ANGLE_LEFT){
            if(fabsf(corners[i].x - (other_object.x + other_object.w)) < x_min_distance)
                x_min_distance = fabsf(corners[i].x - (other_object.x + other_object.w));
            if(fabsf(corners[i].y - other_object.y) < y_min_distance)
                y_min_distance = fabsf(corners[i].y - other_object.y);
        }
        else if(angle_degrees > PROJECTILE_ANGLE_LEFT && angle_degrees < PROJECTILE_ANGLE_UP){
            if(fabsf(corners[i].x - (other_object.x + other_object.w)) < x_min_distance)
                x_min_distance = fabsf(corners[i].x - (other_object.x + other_object.w));
            if(fabsf(corners[i].y - (other_object.y + other_object.h)) < y_min_distance)
                y_min_distance = fabsf(corners[i].y - (other_object.y + other_object.h));
        }
        else if(angle_degrees > PROJECTILE_ANGLE_UP && angle_degrees < PROJECTILE_ANGLE_RIGHT){
            if(fabsf(corners[i].x - other_object.x) < x_min_distance)
                x_min_distance = fabsf(corners[i].x - other_object.x);
            if(fabsf(corners[i].y - (other_object.y + other_object.h)) < y_min_distance)
                y_min_distance = fabsf(corners[i].y - (other_object.y + other_object.h));
        }
    }

    if(x_min_distance < y_min_distance)
        pProjectile->angle_degrees = 180 - pProjectile->angle_degrees;
    else if(y_min_distance < x_min_distance)
        pProjectile->angle_degrees = -pProjectile->angle_degrees;

    normalize_angle(pProjectile);
}

void normalize_angle(Projectile* pProjectile){
    while(pProjectile->angle_degrees < 0.0f)
        pProjectile->angle_degrees += 360.0f;
    while(pProjectile->angle_degrees >= 360.0f)
        pProjectile->angle_degrees -= 360.0f;
}

void destroy_projectile(Projectile* pProjectile){
    if(pProjectile->pTexture)
        SDL_DestroyTexture(pProjectile->pTexture);
    free(pProjectile);
}