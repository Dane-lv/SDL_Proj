#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include "../include/maze.h"
#include "../include/constants.h"
#include "../include/camera.h"

#define MAX_WALLS 100

struct wall {
    SDL_Rect rect;
};

struct maze {
    SDL_Renderer* pRenderer;
    SDL_Texture* wallTexture;
    Wall walls[MAX_WALLS];
    int wallCount;
};

Maze* createMaze(SDL_Renderer* pRenderer, SDL_Texture* wallTexture) {
    Maze* pMaze = malloc(sizeof(struct maze));
    if (!pMaze) {
        printf("Error: Failed to allocate memory for maze\n");
        return NULL;
    }
    
    pMaze->pRenderer = pRenderer;
    pMaze->wallTexture = wallTexture;
    pMaze->wallCount = 0;
    
    return pMaze;
}

void destroyMaze(Maze* pMaze) {
    if (pMaze) {
        free(pMaze);
    }
}

void drawMaze(Maze* pMaze, Camera* pCamera) {
    if (!pMaze) return;
    
    for (int i = 0; i < pMaze->wallCount; i++) {
        // Create a copy of the wall rect for camera adjustment
        SDL_Rect wallRect = pMaze->walls[i].rect;
        
        // Transform world coordinates to screen coordinates
        SDL_Rect adjustedRect = getWorldCoordinatesFromCamera(pCamera, wallRect);
        
        // Only draw walls that are visible on screen
        if (adjustedRect.x < WINDOW_WIDTH && adjustedRect.y < WINDOW_HEIGHT && 
            adjustedRect.x + adjustedRect.w > 0 && adjustedRect.y + adjustedRect.h > 0) {
            // Render the wall
            SDL_RenderCopy(pMaze->pRenderer, pMaze->wallTexture, NULL, &adjustedRect);
        }
    }
}

bool checkCollision(Maze* pMaze, SDL_Rect playerRect) {
    if (!pMaze) return false;
    
    for (int i = 0; i < pMaze->wallCount; i++) {
        if (SDL_HasIntersection(&playerRect, &(pMaze->walls[i].rect))) {
            return true;
        }
    }
    
    return false;
}

void addWall(Maze* pMaze, int x, int y, int width, int height) {
    if (!pMaze || pMaze->wallCount >= MAX_WALLS) return;
    
    Wall newWall;
    newWall.rect.x = x;
    newWall.rect.y = y;
    newWall.rect.w = width;
    newWall.rect.h = height;
    
    pMaze->walls[pMaze->wallCount] = newWall;
    pMaze->wallCount++;
}

void clearWalls(Maze* pMaze) {
    if (!pMaze) return;
    pMaze->wallCount = 0;
}

SDL_Texture* initiateMap(SDL_Renderer *pRenderer)
{
    SDL_Surface *pSurface = IMG_Load("resources/Bakgrund.jpg");
    SDL_Texture *bgTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    return bgTexture;
}

SDL_Texture* initiateMaze(SDL_Renderer *pRenderer)
{
    SDL_Surface *pSurface = IMG_Load("resources/WallTexture.png");
    SDL_Texture *wallTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    return wallTexture;
}

void drawMap(SDL_Renderer *pRenderer, SDL_Texture *bgTexture, Camera *pCamera)
{
    // Create a background rect that fills the viewport
    SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    
    // Set a solid black background
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255); // Pure black
    SDL_RenderFillRect(pRenderer, &bgRect);
}

void destroyTexture(SDL_Texture *texture)
{
    SDL_DestroyTexture(texture);
    texture = NULL;
}