// SDL library for graphics, events, and other game functionality
#include <SDL.h>
// SDL_image for loading images and textures
#include <SDL_image.h>
// standard math library for mathematical functions
#include <math.h>
// for FLT_MAX, used for floating-point comparisons
#include <float.h>
// for boolean type and logic
#include <stdbool.h>

// define M_PI if not already defined (useful for systems that don't provide it by default)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// include project-specific header files for constants, projectile handling, rotated bounding box functionality
#include "../include/constants.h"
#include "../include/projectile.h"
#include "../include/rectangle_obb_corners.h"

// define the structure for a projectile
struct projectile{
    float pos_x, pos_y, angle_degrees, velocity;            // position, angle, and velocity of the projectile
    SDL_Rect bounding_box;                                  // bounding box for collision detection, rendering
    SDL_Texture* pTexture;                                  // texture used to draw the projectile
}; typedef struct projectile Projectile;

// function to create a new projectile with initial position, angle, and texture
Projectile* create_projectile(SDL_Renderer* pRenderer, const float x_init, const float y_init,
        const float angle_degrees_init){
    // allocate memory for the new projectile
    Projectile* pProjectile = malloc(sizeof(Projectile));
    // check if memory allocation succeeded
    if(!pProjectile){
        // print error if memory allocation fails
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }

    // intialize projectile properties
    pProjectile->pos_x = x_init;
    pProjectile->pos_y = y_init;
    pProjectile->angle_degrees = angle_degrees_init;
    // normalize the angle to be within 0 to 360 degrees
    normalize_angle(pProjectile);

    // set default width and height for the projectile's bounding box
    pProjectile->bounding_box.w = PROJECTILE_WIDTH;
    pProjectile->bounding_box.h = PROJECTILE_HEIGHT;

    // set initial velocity for the projectile
    pProjectile->velocity = PROJECTILE_VELOCITY_INIT;

    // create a surface for the projectile texture (currently a white rectangle)
    SDL_Surface* pSurface = SDL_CreateRGBSurface(0, 100, 100, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);//IMG_Load("resources/xxx.png");
    // check if surface creation succeeded
    if(!pSurface){
        // print error if surface creation fails
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }

    // fill the surface with a white color (for now, can be replaced with an image)
    SDL_FillRect(pSurface, NULL, SDL_MapRGB(pSurface->format, 255, 255, 255));

    // create a texture from the surface and store it in the projectile structure
    (pProjectile->pTexture) = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    // free the surface after the texture is created
    SDL_FreeSurface(pSurface);
    // check if texture creation succeeded
    if(!(pProjectile->pTexture)){
        // print error if texture creation fails
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    
    // return the created projectile object
    return pProjectile;
}

// function to render the projectile on the screen
// it draws the projectile's texture at its current position and angle
void draw_projectile(SDL_Renderer* pRenderer, Projectile* pProjectile){
    // draw the texture with rotation
    SDL_RenderCopyEx(pRenderer, pProjectile->pTexture, NULL, &(pProjectile->bounding_box),
        pProjectile->angle_degrees, NULL, SDL_FLIP_NONE);
}

// function to update the projectile's position based on its velocity and the time delta
void update_projectile(Projectile* pProjectile, const float delta_time){
    // convert angle to radians
    float angle_radians = (pProjectile->angle_degrees * M_PI) / 180.0f;
    // calculate the movement components based on velocity and angle
    float dx = pProjectile->velocity * cos(angle_radians) * delta_time;
    float dy = pProjectile->velocity * sin(angle_radians) * delta_time;

    // update the projectile's position
    pProjectile->pos_x += dx;
    pProjectile->pos_y += dy;

    // update the bounding box to match the new position
    pProjectile->bounding_box.x = pProjectile->pos_x;
    pProjectile->bounding_box.y = pProjectile->pos_y;

    // keep the projectile within screen bounds (clamp position to window size)
    if(pProjectile->bounding_box.x < 0)
        pProjectile->bounding_box.x = 0;
    else if(pProjectile->bounding_box.x > (WINDOW_WIDTH - PROJECTILE_WIDTH))
        pProjectile->bounding_box.x = WINDOW_WIDTH - PROJECTILE_WIDTH;

    if(pProjectile->bounding_box.y < 0)
        pProjectile->bounding_box.y = 0;
    else if(pProjectile->bounding_box.y > (WINDOW_HEIGHT - PROJECTILE_HEIGHT))
        pProjectile->bounding_box.y = WINDOW_HEIGHT - PROJECTILE_HEIGHT;
    
    // ensure that the floating point position matches the integer bounding box position
    if((int) pProjectile->pos_x != pProjectile->bounding_box.x)
        pProjectile->pos_x = pProjectile->bounding_box.x;
    if((int) pProjectile->pos_y != pProjectile->bounding_box.y)
        pProjectile->pos_y = pProjectile->bounding_box.y;
}

// function to handle collision resolution between the projectile and another object
void resolve_projectile_collision(Projectile* pProjectile, const SDL_Rect other_object,
        const Object_ID other_object_ID){
    SDL_FPoint corners[4];
    // get the rotated corners of the projectile's bounding box
    corners_rotate(pProjectile->bounding_box, pProjectile->angle_degrees, corners);

    bool overlap = false;
    // check if any of the projectile's corners overlap with the other object's bounding box
    for(int i = 0; i < 4; i++){
        if(corners_overlap(corners[i], other_object))
            overlap = true;
    }

    // if no overlap, do nothing
    if(!overlap)
        return;
    
    // handle collision with player or wall
    if(other_object_ID == OBJECT_ID_PLAYER){
        // handle player-specific collision logic (not implemented here)
    }
    else if(other_object_ID == OBJECT_ID_WALL){
        // handle wall collision and reflect the projectile's angle
        float angle_degrees = pProjectile->angle_degrees;
        if(fmodf(angle_degrees, 90.0f) == 0){
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

        // calculate the minimum distance to determine the proper reflection angle
        float x_min_distance = FLT_MAX / 2.0f;
        float y_min_distance = FLT_MAX / 2.0f;
        for(int i = 0; i < 4; i++){
            // determine reflection based on the angle of the projectile
            // this involves calculating distances between corners and the wall
            if(angle_degrees > PROJECTILE_ANGLE_RIGHT && angle_degrees < PROJECTILE_ANGLE_DOWN){
                if(fabsf(corners[i].x - other_object.x) < x_min_distance)
                    x_min_distance = fabsf(corners[i].x - other_object.x);
                if(fabsf(corners[i].y - other_object.y) < y_min_distance)
                    y_min_distance = fabsf(corners[i].y - other_object.y);
            }
            // other conditions omitted for brevity...
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

        // reflect the angle based on the closest distance
        if(x_min_distance < y_min_distance)
            pProjectile->angle_degrees = 180 - pProjectile->angle_degrees;
        else if(y_min_distance < x_min_distance)
            pProjectile->angle_degrees = -pProjectile->angle_degrees;

        // normalize the angle to be within 0 to 360 degrees after reflection
        normalize_angle(pProjectile);
    }
}

// function to normalize the angle of the projectile to be within 0 to 360 degrees
void normalize_angle(Projectile* pProjectile){
    while(pProjectile->angle_degrees < 0.0f)
        pProjectile->angle_degrees += 360.0f;               // wrap angle around if it's negative
    while(pProjectile->angle_degrees >= 360.0f)
        pProjectile->angle_degrees -= 360.0f;               // wrap angle around if it's greater than 360
}

// function to destroy a projectile and free its allocated resources
void destroy_projectile(Projectile* pProjectile){
    if(pProjectile->pTexture)
        SDL_DestroyTexture(pProjectile->pTexture);          // free the projectile's texture
    free(pProjectile);                                      // free the projectile's memory
}