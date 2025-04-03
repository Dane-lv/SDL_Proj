#include <SDL.h>
#include <SDL_image.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../include/constants.h"
#include "../include/projectile.h"

struct projectile{
    float pos_x, pos_y, angle, velocity;
    SDL_Rect bounding_box;
    SDL_Texture *pTexture;
}; typedef struct projectile Projectile;

Projectile *createProjectile(float x_init, float y_init, float angle_init, SDL_Renderer *pRenderer){
    Projectile *pProjectile = malloc(sizeof(Projectile));

    pProjectile->pos_x = x_init;
    pProjectile->pos_y = y_init;
    pProjectile->angle = angle_init;

    pProjectile->bounding_box.w = PROJECTILE_WIDTH;
    pProjectile->bounding_box.h = PROJECTILE_HEIGHT;

    pProjectile->velocity = PROJECTILE_VELOCITY_INIT;

    SDL_Surface *pSurface = SDL_CreateRGBSurface(0, 100, 100, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);//IMG_Load("resources/xxx.png");
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

void drawProjectile(SDL_Renderer *pRenderer, Projectile *pProjectile){
    SDL_RenderCopyEx(pRenderer, pProjectile->pTexture, NULL, &(pProjectile->bounding_box),
        pProjectile->angle, NULL, SDL_FLIP_NONE);
}

void updateProjectile(Projectile *pProjectile, float delta_time){
    float radians = (pProjectile->angle * M_PI) / 180.0f; // Omvandla till radianer
    float dx = pProjectile->velocity * cos(radians) * delta_time;    // Beräkna hastighetskomponenter
    float dy = pProjectile->velocity * sin(radians) * delta_time;

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

void resolveProjectileCollision(Projectile *pProjectile, SDL_Rect other_object){
    if(!SDL_HasIntersection(&(pProjectile->bounding_box), &other_object))
        return;

    float proj_x = pProjectile->bounding_box.x;
    float proj_y = pProjectile->bounding_box.y;
    float proj_w = pProjectile->bounding_box.w;
    float proj_h = pProjectile->bounding_box.h;
        
    float wall_x = other_object.x;
    float wall_y = other_object.y;
    float wall_w = other_object.w;
    float wall_h = other_object.h;
        
    // Beräkna överlappning för varje sida
    float overlap_top = fabs((proj_y + proj_h) - wall_y);
    float overlap_bottom = fabs(proj_y - (wall_y + wall_h));
    float overlap_left = fabs((proj_x + proj_w) - wall_x);
    float overlap_right = fabs(proj_x - (wall_x + wall_w));
        
    // Hitta minsta överlappning → den sida som träffades
    float min_overlap = fmin(fmin(overlap_top, overlap_bottom), fmin(overlap_left, overlap_right));
        
    float normal_x = 0.0f;
    float normal_y = 0.0f;

    if(min_overlap == overlap_top)                  // Träffade OVANIFRÅN → normalvektor (0, -1)
        normal_y = -1.0f;
    else if(min_overlap == overlap_bottom)          // Träffade UNDERIFRÅN → normalvektor (0, 1)
        normal_y = 1.0f;
    else if(min_overlap == overlap_left)            // Träffade FRÅN VÄNSTER → normalvektor (-1, 0)
        normal_x = -1.0f;
    else if(min_overlap == overlap_right)           // Träffade FRÅN HÖGER → normalvektor (1, 0)
        normal_x = 1.0f;
        
    // Nu kan vi reflektera hastigheten precis som innan
    float radians = (pProjectile->angle * M_PI) / 180.0f;
    float vel_x = cos(radians);
    float vel_y = sin(radians);
        
    float dotProduct = (vel_x * normal_x) + (vel_y * normal_y);
    float reflected_x = vel_x - 2 * dotProduct * normal_x;
    float reflected_y = vel_y - 2 * dotProduct * normal_y;
        
    // Uppdatera vinkeln för reflektionen
    pProjectile->angle = atan2(reflected_y, reflected_x) * 180.0f / M_PI;
}

void destroyProjectile(Projectile *pProjectile){
    if(pProjectile->pTexture)
        SDL_DestroyTexture(pProjectile->pTexture);
    free(pProjectile);
}