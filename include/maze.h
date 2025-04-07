#ifndef MAZE_H
#define MAZE_H

#include <SDL.h>
#include <stdbool.h>

// Forward declarations
typedef struct camera Camera;
typedef struct maze Maze;
typedef struct wall Wall;

Maze* createMaze(SDL_Renderer* pRenderer, SDL_Texture* wallTexture);
void destroyMaze(Maze* pMaze);
void drawMaze(Maze* pMaze, Camera* pCamera);
bool checkCollision(Maze* pMaze, SDL_Rect playerRect);
void addWall(Maze* pMaze, int x, int y, int width, int height);
void clearWalls(Maze* pMaze);

SDL_Texture* initiateMap(SDL_Renderer* pRenderer);
SDL_Texture* initiateMaze(SDL_Renderer* pRenderer);
void drawMap(SDL_Renderer* pRenderer, SDL_Texture* bgTexture, Camera* pCamera);
void destroyTexture(SDL_Texture* texture);

void mazeLayout1(Maze* pMaze);

#endif