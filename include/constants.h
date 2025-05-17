#ifndef CONSTANTS_H
#define CONSTANTS_H

/* ----------------------------------------------------------
 *  Fönster
 * -------------------------------------------------------- */
#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  600

/* ----------------------------------------------------------
 *  Tile-system
 * -------------------------------------------------------- */
#define TILE_SIZE   32          /* pixlar per ruta           */

/* --------------- VIKTIGT ----------------------------------
 *  Ställ in hur många rutor labyrinten ska ha.
 *  Dessa var de värden du körde med när kartan var ”stor”.
 * -------------------------------------------------------- */
#define TILE_WIDTH   50     
#define TILE_HEIGHT  40         

/* --------------- Beräknad världsstorlek ------------------ */
#define WORLD_WIDTH   (TILE_WIDTH  * TILE_SIZE)   /* = 1920  */
#define WORLD_HEIGHT  (TILE_HEIGHT * TILE_SIZE)   /* = 1600  */

/* ----------------------------------------------------------
 *  Spelar‐ och spelkonstanter
 * -------------------------------------------------------- */
#define MAX_PLAYERS    5
#define PLAYERWIDTH   30
#define PLAYERHEIGHT  45
#define PLAYERSPEED   200

#define CAMERA_ZOOM   1.5f
#define SPECTATE_ZOOM 0.6f

#define MAX_WALLS     100
#define FOG_MAX_DIST  200.0f
#define FOG_MIN_BRIGHTNESS 0.10f

#define PROJSPEED       400
#define MAX_PROJECTILES 10

/* ----------------------------------------------------------
 *  Objekt-ID
 * -------------------------------------------------------- */
typedef enum {
    OBJECT_ID_PLAYER,
    OBJECT_ID_WALL,
    OBJECT_ID_PROJECTILE
} Object_ID;

#endif /* CONSTANTS_H */