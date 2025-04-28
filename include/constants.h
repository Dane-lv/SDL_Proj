#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define PLAYERWIDTH 30
#define PLAYERHEIGHT 45
#define PLAYERSPEED 200
#define WORLD_WIDTH 1600
#define WORLD_HEIGHT 1200
#define CAMERA_ZOOM 1.5
#define TILE_WIDTH 30
#define TILE_HEIGHT 25
#define TILE_SIZE 32   
#define MAX_WALLS 100  
#define FOG_MAX_DIST 200.0f   
#define FOG_MIN_BRIGHTNESS 0.10f 
#define PROJSPEED 400
#define MAX_PROJECTILES 10

typedef enum {
    OBJECT_ID_PLAYER,
    OBJECT_ID_WALL,
    OBJECT_ID_PROJECTILE
} Object_ID;

#endif 
