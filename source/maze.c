#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>

#include "../include/maze.h"
#include "../include/constants.h"
#include "../include/camera.h"
#include "../include/player.h"

#define BASE_W 30
#define BASE_H 25

static const int BASE_WALLS[][4] = {
    {  5,  5, 10,  5}, { 10,  0, 10, 10}, {  8, 11, 12, 11},
    {  5,  8,  5, 15}, {  0, 16,  8, 16}, { 12, 16, 15, 16},
    { 16,  8, 16, 20}, { 16,  3, 16,  5}, { 13,  3, 22,  3},
    { 20,  4, 20, 10}, { 20, 11, 23, 11}, { 24, 11, 24, 14},
    { 24, 18, 24, 24}, { 20, 19, 24, 19}, {  5, 20, 11, 20},
    { 12, 17, 12, 21}, { 25,  5, 25,  8}, { 25,  8, 29,  8}
};
static const size_t BASE_WALL_COUNT =
        sizeof(BASE_WALLS) / sizeof(BASE_WALLS[0]);

struct maze
{
    SDL_Renderer *pRenderer;
    SDL_Texture *tileMapTexture;
    SDL_Surface *tileMapSurface;
    int tiles[TILE_WIDTH][TILE_HEIGHT];
    SDL_Rect tileRect;
    Object_ID objectID;
};

Maze *createMaze(SDL_Renderer *r, SDL_Texture *t, SDL_Surface *s)
{
    Maze *m = malloc(sizeof *m);
    if (!m)
    {
        printf("Maze malloc failed\n");
        return NULL;
    }

    m->pRenderer = r;
    m->tileMapTexture = t;
    m->tileMapSurface = s;
    m->objectID = OBJECT_ID_WALL;
    return m;
}

void destroyMaze(Maze *m) { free(m); }

void generateMazeLayout(Maze *m)
{
    for (int x = 0; x < TILE_WIDTH; ++x)
        for (int y = 0; y < TILE_HEIGHT; ++y)
            m->tiles[x][y] =
                (x == 0 || x == TILE_WIDTH - 1 ||
                 y == 0 || y == TILE_HEIGHT - 1)
                    ? 2
                    : 1;

    for (int ox = 0; ox < TILE_WIDTH; ox += BASE_W)
        for (int oy = 0; oy < TILE_HEIGHT; oy += BASE_H)
        {
            for (size_t i = 0; i < BASE_WALL_COUNT; ++i)
            {
                int x1 = BASE_WALLS[i][0] + ox;
                int y1 = BASE_WALLS[i][1] + oy;
                int x2 = BASE_WALLS[i][2] + ox;
                int y2 = BASE_WALLS[i][3] + oy;

                if (x1 <= 0 || x2 >= TILE_WIDTH - 1 ||
                    y1 <= 0 || y2 >= TILE_HEIGHT - 1)
                    continue;

                if (y1 == y2)
                {
                    if (x1 > x2)
                    {
                        int t = x1;
                        x1 = x2;
                        x2 = t;
                    }
                    for (int x = x1; x <= x2; ++x)
                        m->tiles[x][y1] = 2;
                }
                else if (x1 == x2)
                {
                    if (y1 > y2)
                    {
                        int t = y1;
                        y1 = y2;
                        y2 = t;
                    }
                    for (int y = y1; y <= y2; ++y)
                        m->tiles[x1][y] = 2;
                }
            }
        }
}

bool checkCollision(Maze *m, SDL_Rect r)
{
    int l = r.x / TILE_SIZE;
    int rgt = (r.x + r.w - 1) / TILE_SIZE;
    int t = r.y / TILE_SIZE;
    int b = (r.y + r.h - 1) / TILE_SIZE;

    for (int x = l; x <= rgt; ++x)
        for (int y = t; y <= b; ++y)
            if (x >= 0 && x < TILE_WIDTH &&
                y >= 0 && y < TILE_HEIGHT &&
                m->tiles[x][y] == 2)
                return true;
    return false;
}

void initiateMap(Maze *m)
{
    m->tileMapSurface = SDL_LoadBMP("resources/Tiles.bmp");
    m->tileMapTexture =
        SDL_CreateTextureFromSurface(m->pRenderer, m->tileMapSurface);
    SDL_FreeSurface(m->tileMapSurface);
}

void addWall(Maze *m, int x1, int y1, int x2, int y2)
{
    if (y1 == y2)
    {
        if (x1 > x2)
        {
            int t = x1;
            x1 = x2;
            x2 = t;
        }
        for (int x = x1; x <= x2; ++x)
            if (x > 0 && x < TILE_WIDTH - 1 &&
                y1 > 0 && y1 < TILE_HEIGHT - 1)
                m->tiles[x][y1] = 2;
    }
    else if (x1 == x2)
    {
        if (y1 > y2)
        {
            int t = y1;
            y1 = y2;
            y2 = t;
        }
        for (int y = y1; y <= y2; ++y)
            if (x1 > 0 && x1 < TILE_WIDTH - 1 &&
                y > 0 && y < TILE_HEIGHT - 1)
                m->tiles[x1][y] = 2;
    }
}

void drawMap(Maze *m, Camera *c, Player *p, bool spectate)
{
    SDL_Rect pr = getPlayerRect(p);
    float px = pr.x + pr.w * 0.5f;
    float py = pr.y + pr.h * 0.5f;

    for (int x = 0; x < TILE_WIDTH; ++x)
    {
        for (int y = 0; y < TILE_HEIGHT; ++y)
        {

            int wx = x * TILE_SIZE;
            int wy = y * TILE_SIZE;

            Uint8 wallR, wallG, wallB;
            Uint8 floorR, floorG, floorB;

            if (spectate)
            {
                wallR = 0;
                wallG = 255;
                wallB = 255;
                floorR = 50;
                floorG = 50;
                floorB = 70;
            }
            else
            {
                float cx = wx + TILE_SIZE * 0.5f;
                float cy = wy + TILE_SIZE * 0.5f;
                float dx = cx - px;
                float dy = cy - py;
                float dist = sqrtf(dx * dx + dy * dy);
                float t = 1.f - dist / FOG_MAX_DIST;
                if (t < 0.f)
                    t = 0.f;
                float b = FOG_MIN_BRIGHTNESS + t * (1.f - FOG_MIN_BRIGHTNESS);

                wallR = (Uint8)(0 * b);
                wallG = (Uint8)(255 * b);
                wallB = (Uint8)(255 * b);
                floorR = (Uint8)(50 * b);
                floorG = (Uint8)(50 * b);
                floorB = (Uint8)(70 * b);
            }

            m->tileRect = (SDL_Rect){wx, wy, TILE_SIZE, TILE_SIZE};
            SDL_Rect adj = getWorldCoordinatesFromCamera(c, m->tileRect);

            SDL_SetRenderDrawColor(m->pRenderer,
                                   m->tiles[x][y] == 2 ? wallR : floorR,
                                   m->tiles[x][y] == 2 ? wallG : floorG,
                                   m->tiles[x][y] == 2 ? wallB : floorB,
                                   255);
            SDL_RenderFillRect(m->pRenderer, &adj);
        }
    }
}