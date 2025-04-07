#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define PLAYERWIDTH 50
#define PLAYERHEIGHT 50
#define PLAYERSPEED 200

// ----------------------------------------- used for projectile.c ------------------------------------------
// define the initial velocity of the projectile
#define PROJECTILE_VELOCITY_INIT 50.0f
// define the width of the projectile
#define PROJECTILE_WIDTH 100.0f
// define the height of the projectile
#define PROJECTILE_HEIGHT 100.0f
// define the angle for a projectile moving to the right (0 degrees)
#define PROJECTILE_ANGLE_RIGHT 0.0f
// define the angle for a projectile moving downwards (90 degrees)
#define PROJECTILE_ANGLE_DOWN 90.0f
// define the angle for a projectile moving to the left (180 degrees)
#define PROJECTILE_ANGLE_LEFT 180.0f
// define the angle for a projectile moving upward (270 degrees)
#define PROJECTILE_ANGLE_UP 270.0f

// define an enumeration for different object types (used for collision detection)
typedef enum{
    OBJECT_ID_PLAYER,                       // OBJECT_ID_PLAYER: represent a player object
    OBJECT_ID_WALL,                         // OBJECT_ID_WALL: represent a wall object
    OBJECT_ID_PROJECTILE                    // OBJECT_ID_PROJECTILE: represents a projectile object
} Object_ID;

#endif 
