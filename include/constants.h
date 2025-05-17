#ifndef CONSTANTS_H
#define CONSTANTS_H

/* ---------------------------------------------------------- */
/* Fönster‐, värld- och tile-storlekar                        */
/* ---------------------------------------------------------- */

#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  600

/* ---------- NY MAZE-STORLEK -------------------------------- */
#define TILE_WIDTH     50        /* NYTT */
#define TILE_HEIGHT    40        /* NYTT */
#define TILE_SIZE      32

/*  Världen anpassas alltid till maze-storleken (inga globala   */
/*  variabler behövs – bara makron)                             */
#define WORLD_WIDTH    (TILE_WIDTH  * TILE_SIZE)   /* NYTT */
#define WORLD_HEIGHT   (TILE_HEIGHT * TILE_SIZE)   /* NYTT */

/* ---------------------------------------------------------- */
/* Gameplay-konstanter                                        */
/* ---------------------------------------------------------- */
#define MAX_PLAYERS     5
#define PLAYERWIDTH    30
#define PLAYERHEIGHT   45
#define PLAYERSPEED   200

#define CAMERA_ZOOM    1.5
#define SPECTATE_ZOOM  0.6f

#define FOG_MAX_DIST        200.0f
#define FOG_MIN_BRIGHTNESS    0.10f

#define PROJSPEED       400
#define MAX_PROJECTILES  10

typedef enum {
    OBJECT_ID_PLAYER,
    OBJECT_ID_WALL,
    OBJECT_ID_PROJECTILE
} Object_ID;

#endif 