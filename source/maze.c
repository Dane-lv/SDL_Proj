#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_image.h>
#include "../include/maze.h"
#include "../include/constants.h"
#include "../include/camera.h"

#define MAX_WALLS 100

struct maze {
    SDL_Renderer* pRenderer;
    SDL_Texture* tileMapTexture;
    SDL_Surface* tileMapSurface;
    int tiles[TILE_WIDTH][TILE_HEIGHT];
    SDL_Rect tileRect;
    Object_ID objectID;
};

Maze* createMaze(SDL_Renderer* pRenderer, SDL_Texture* tileMapTexture, SDL_Surface* tileMapSurface) 
{
    Maze* pMaze = malloc(sizeof(struct maze));
    if (!pMaze) {
        printf("Error: Failed to allocate memory for maze\n");
        return NULL;
    }
    
    pMaze->pRenderer = pRenderer;
    pMaze->tileMapSurface = tileMapSurface;
    pMaze->tileMapTexture = tileMapTexture;
    pMaze->objectID = OBJECT_ID_WALL;
    
    return pMaze;
}

void generateMazeLayout(Maze* pMaze)
{
    // 0 = golv, 1 = vägg
    for (int x = 0; x < TILE_WIDTH; x++)
    {
        for (int y = 0; y < TILE_HEIGHT; y++)
        {
            // Gör kanterna till vägg
            if (x == 0 || x == TILE_WIDTH-1 || y == 0 || y == TILE_HEIGHT-1) {
                pMaze->tiles[x][y] = 2; // "2" kanske betyder vägg
            } else {
                pMaze->tiles[x][y] = 1; // "1" kanske betyder golv
            }
        }
    }
}

void destroyMaze(Maze* pMaze) 
{
    if (pMaze) {
        free(pMaze);
    }
}

bool checkCollision(Maze* pMaze, SDL_Rect playerRect)
{
    // Räkna ut vilka tile-koordinater spelarens rekt täcker
    int leftTile   = playerRect.x / 32;
    int rightTile  = (playerRect.x + playerRect.w - 1) / 32;
    int topTile    = playerRect.y / 32;
    int bottomTile = (playerRect.y + playerRect.h - 1) / 32;

    // Loopa över de tiles som spelaren täcker:
    for (int tx = leftTile; tx <= rightTile; tx++)
    {
        for (int ty = topTile; ty <= bottomTile; ty++)
        {
            // Se om denna tile existerar inom arrayens gränser
            if (tx >= 0 && tx < TILE_WIDTH && ty >= 0 && ty < TILE_HEIGHT)
            {
                // Kolla om det är en vägg
                if (pMaze->tiles[tx][ty] == 2) {
                    return true; 
                }
            }
        }
    }
    return false;
}


void initiateMap(Maze* pMaze)
{
    pMaze->tileMapSurface = SDL_LoadBMP("resources/Tiles.bmp");
    pMaze->tileMapTexture = SDL_CreateTextureFromSurface(pMaze->pRenderer, pMaze->tileMapSurface);
    SDL_FreeSurface(pMaze->tileMapSurface);
}

void addWall(Maze* pMaze, int x1, int y1, int x2, int y2)
{
    // Horisontell linje: y1 == y2
    if (y1 == y2) {
        // Se till att x1 <= x2
        if (x1 > x2) {
            int tmp = x1; 
            x1 = x2; 
            x2 = tmp;
        }
        for (int x = x1; x <= x2; x++) {
            pMaze->tiles[x][y1] = 2; // 2 = vägg
        }
    }
    else if (x1 == x2) {
        // Se till att y1 <= y2
        if (y1 > y2) {
            int tmp = y1; 
            y1 = y2; 
            y2 = tmp;
        }
        for (int y = y1; y <= y2; y++) {
            pMaze->tiles[x1][y] = 2; // 2 = vägg
        }
    }
}

void drawMap(Maze* pMaze, Camera* pCamera)
{
    for (int x = 0; x < TILE_WIDTH; x++)
    {
        for (int y = 0; y < TILE_HEIGHT; y++)
        {
            int worldX = x * 32;
            int worldY = y * 32;
            pMaze->tileRect.x = worldX;
            pMaze->tileRect.y = worldY;
            pMaze->tileRect.w = 30;
            pMaze->tileRect.h = 30;
            addWall(pMaze,  5,  5, 10,  5); // Horisontell
            addWall(pMaze, 10,  0, 10, 10); // Vertikal
            addWall(pMaze,  8, 11, 12, 11); // Horisontell
            addWall(pMaze,  5,  8,  5, 15); // Vertikal
            addWall(pMaze,  0, 16,  8, 16); // ...
            addWall(pMaze, 12, 16, 15, 16);
            addWall(pMaze, 16,  8, 16, 20);
            addWall(pMaze, 16,  3, 16,  5);
            addWall(pMaze, 13,  3, 22,  3);
            addWall(pMaze, 20,  4, 20, 10);
            addWall(pMaze, 20, 11, 23, 11);
            addWall(pMaze, 24, 11, 24, 14);
            addWall(pMaze, 24, 18, 24, 24);
            addWall(pMaze, 20, 19, 24, 19);
            addWall(pMaze,  5, 20, 11, 20);
            addWall(pMaze, 12, 17, 12, 21);
            addWall(pMaze, 25,  5, 25,  8);
            addWall(pMaze, 25,  8, 29,  8);
            // Konvertera från världskoordinater till skärmkoordinater via kameran
            SDL_Rect adjustedRect = getWorldCoordinatesFromCamera(pCamera, pMaze->tileRect);
            if (pMaze->tiles[x][y] == 2) {
                // Vägg
                SDL_SetRenderDrawColor(pMaze->pRenderer, 255, 255, 255, 255);
            }
            else {
                // Golv
                SDL_SetRenderDrawColor(pMaze->pRenderer, 50, 50, 50, 255);
            }

            // Rendera "fylld rektangel" (eller SDL_RenderCopy om du har en textur)
            SDL_RenderFillRect(pMaze->pRenderer, &adjustedRect);
        }
    }
}



