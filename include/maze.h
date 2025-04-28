#ifndef MAZE_H
#define MAZE_H

#include <SDL.h>
#include <stdbool.h>
#include "../include/player.h"
#include "constants.h"

// Forward declarations
typedef struct camera Camera;
typedef struct maze Maze;

Maze* createMaze(SDL_Renderer* pRenderer, SDL_Texture* tileMapTexture, SDL_Surface* tileMapSurface);
void destroyMaze(Maze* pMaze);
bool checkCollision(Maze* pMaze, SDL_Rect playerRect);
void generateMazeLayout(Maze* pMaze);
void addWall(Maze* pMaze, int x1, int y1, int x2, int y2);
void initiateMap(Maze* pMaze);
void drawMap(Maze* pMaze, Camera* pCamera, Player* pPlayer);


#endif