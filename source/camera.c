#include <stdlib.h>
#include <math.h>
#include <SDL.h>

#include "../include/camera.h"
#include "../include/constants.h"
#include "../include/player.h"

/* ---------------------------------------------------------- */
struct camera {
    float x, y;
    int   width, height;
    float zoom;
};

/* ========================================================== */
Camera *createCamera(int width, int height)
{
    Camera *c = malloc(sizeof *c);
    c->x = c->y = 0;
    c->width  = width;
    c->height = height;
    c->zoom   = CAMERA_ZOOM;
    return c;
}

/* ---------------------------------------------------------- */
/*  Auto-zoom som passar exakt inom fönstret                  */
static float calcSpectateZoom(int camW, int camH)
{
    float zx = (float)camW / WORLD_WIDTH;
    float zy = (float)camH / WORLD_HEIGHT;
    return fminf(zx, zy);
}

/* ========================================================== */
void updateCamera(Camera *c, Player *p)
{
    SDL_Rect pr = getPlayerPosition(p);

    int effW = c->width  / c->zoom;
    int effH = c->height / c->zoom;

    c->x = pr.x + pr.w * 0.5f - effW * 0.5f;
    c->y = pr.y + pr.h * 0.5f - effH * 0.5f;
}

/* ---------------------------------------------------------- */
void setCameraSpectateMode(Camera *c, bool enabled)
{
    if (enabled) {
        c->zoom = calcSpectateZoom(c->width, c->height);

        int effW = c->width  / c->zoom;
        int effH = c->height / c->zoom;
        c->x = WORLD_WIDTH  * 0.5f - effW * 0.5f;
        c->y = WORLD_HEIGHT * 0.5f - effH * 0.5f;
    } else {
        c->zoom = CAMERA_ZOOM;
    }
}

/* ---------------------------------------------------------- */
void setCameraPosition(Camera *c, float x, float y)
{
    int effW = c->width  / c->zoom;
    int effH = c->height / c->zoom;
    c->x = x - effW * 0.5f;
    c->y = y - effH * 0.5f;
}

/* ---------------------------------------------------------- */
SDL_Rect getWorldCoordinatesFromCamera(Camera *c, SDL_Rect r)
{
    SDL_Rect out = r;

    out.x = (int)((r.x - c->x) * c->zoom);
    out.y = (int)((r.y - c->y) * c->zoom);
    out.w = (int)(r.w * c->zoom);
    out.h = (int)(r.h * c->zoom);

    return out;
}

int getCameraX(Camera *c) { return (int)c->x; }
int getCameraY(Camera *c) { return (int)c->y; }

/* ---------------------------------------------------------- */
void destroyCamera(Camera *c) { free(c); }