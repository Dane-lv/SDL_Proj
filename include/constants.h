#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define PLAYERWIDTH 50
#define PLAYERHEIGHT 50
#define PLAYERSPEED 200
#define WORLD_WIDTH 1600
#define WORLD_HEIGHT 1200
#define CAMERA_ZOOM 1.5 

typedef enum {
    OBJECT_ID_PLAYER,
    OBJECT_ID_WALL,
    OBJECT_ID_PROJECTILE
} Object_ID;

#endif 
