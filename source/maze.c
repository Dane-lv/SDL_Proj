#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>     
#include <SDL.h>
#include <SDL_image.h>
#include "../include/maze.h"
#include "../include/constants.h"
#include "../include/camera.h"
#include "../include/player.h"


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
    // 1 = golv, 2 = vägg
    for (int x = 0; x < TILE_WIDTH; x++)
    {
        for (int y = 0; y < TILE_HEIGHT; y++)
        {
            // Gör kanterna till vägg
            if (x == 0 || x == TILE_WIDTH-1 || y == 0 || y == TILE_HEIGHT-1) {
                pMaze->tiles[x][y] = 2; // "2" betyder vägg
            } else {
                pMaze->tiles[x][y] = 1; // "1" betyder golv
            }
        }
    }
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

void drawMap(Maze* pMaze, Camera* pCamera, Player* pPlayer, bool isSpectating)
{
    SDL_Rect pr = getPlayerRect(pPlayer);
    float px = pr.x + pr.w * 0.5f;
    float py = pr.y + pr.h * 0.5f;
    
    // Define colors for spectate mode (full brightness)
    Uint8 spectateWallR = 0;    // Cyan walls
    Uint8 spectateWallG = 255;
    Uint8 spectateWallB = 255;
    
    Uint8 spectateFloorR = 50;  // Dark gray floor
    Uint8 spectateFloorG = 50;
    Uint8 spectateFloorB = 70;
    
    for (int x = 0; x < TILE_WIDTH; x++)
    {
        for (int y = 0; y < TILE_HEIGHT; y++)
        {
            int worldX = x * 32;
            int worldY = y * 32;
            
            // Determine coloring based on spectate mode
            Uint8 wallR, wallG, wallB;
            Uint8 floorR, floorG, floorB;
            
            if (isSpectating) {
                // In spectate mode, use full brightness
                wallR = spectateWallR;
                wallG = spectateWallG;
                wallB = spectateWallB;
                floorR = spectateFloorR;
                floorG = spectateFloorG;
                floorB = spectateFloorB;
            } else {
                // Normal play mode with fog of war
                float cx = worldX + 16;        
                float cy = worldY + 16;
                float dx = cx - px;
                float dy = cy - py;
                float dist = sqrtf(dx*dx + dy*dy);

                float t = 1.0f - dist / FOG_MAX_DIST;      
                if (t < 0.0f) t = 0.0f;
                float b = FOG_MIN_BRIGHTNESS + t * (1.0f - FOG_MIN_BRIGHTNESS);
                
                wallR  = (Uint8)(  0 * b);    // Cyan walls
                wallG  = (Uint8)(255 * b);
                wallB  = (Uint8)(255 * b);
                floorR = (Uint8)( 50 * b);    // Dark gray floor
                floorG = (Uint8)( 50 * b);
                floorB = (Uint8)( 70 * b);
            }
            
            pMaze->tileRect.x = worldX;
            pMaze->tileRect.y = worldY;
            pMaze->tileRect.w = 32;
            pMaze->tileRect.h = 32;
            
            // Convert from world coordinates to screen coordinates via camera
            SDL_Rect adjustedRect = getWorldCoordinatesFromCamera(pCamera, pMaze->tileRect);
            
            if (pMaze->tiles[x][y] == 2) {    // Wall
                SDL_SetRenderDrawColor(pMaze->pRenderer, wallR, wallG, wallB, 255);
            } else {                          // Floor
                SDL_SetRenderDrawColor(pMaze->pRenderer, floorR, floorG, floorB, 255);
            }

            // Render filled rectangle
            SDL_RenderFillRect(pMaze->pRenderer, &adjustedRect);
        }
    }
}



