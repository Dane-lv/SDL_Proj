#include <SDL.h>
#include "../include/camera.h"
#include "../include/constants.h"
#include "../include/player.h"
#include <stdlib.h>

struct camera
{
    float x, y;
    int width, height;
    float zoom;
};

Camera *createCamera(int w, int h)
{
    Camera *c = malloc(sizeof *c);
    c->x = c->y = 0;
    c->width = w;
    c->height = h;
    c->zoom = CAMERA_ZOOM;
    return c;
}
void destroyCamera(Camera *c) { free(c); }

void updateCamera(Camera *c, Player *p)
{
    SDL_Rect pr = getPlayerPosition(p);
    int effW = c->width / c->zoom;
    int effH = c->height / c->zoom;

    c->x = pr.x + pr.w * 0.5f - effW * 0.5f;
    c->y = pr.y + pr.h * 0.5f - effH * 0.5f;
}

void setCameraSpectateMode(Camera *c, bool on)
{
    if (on)
    {
        float zx = (float)c->width / WORLD_WIDTH;
        float zy = (float)c->height / WORLD_HEIGHT;
        float fit = (zx < zy) ? zx : zy;

        c->zoom = fit * 0.85f;

        int effW = c->width / c->zoom;
        int effH = c->height / c->zoom;
        c->x = WORLD_WIDTH * 0.5f - effW * 0.5f;
        c->y = WORLD_HEIGHT * 0.5f - effH * 0.5f;
    }
    else
    {
        c->zoom = CAMERA_ZOOM;
    }
}

void setCameraPosition(Camera *c, float x, float y)
{
    int effW = c->width / c->zoom;
    int effH = c->height / c->zoom;
    c->x = x - effW * 0.5f;
    c->y = y - effH * 0.5f;
}

SDL_Rect getWorldCoordinatesFromCamera(Camera *c, SDL_Rect r)
{
    SDL_Rect out = r;
    out.x = (int)((r.x - c->x) * c->zoom);
    out.y = (int)((r.y - c->y) * c->zoom);
    out.w = (int)(r.w * c->zoom);
    out.h = (int)(r.h * c->zoom);
    return out;
}