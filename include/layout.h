#ifndef LAYOUT_H
#define LAYOUT_H

#include <SDL.h>

SDL_Texture* initiateMap(SDL_Renderer *pRenderer);
SDL_Texture* initiateMaze(SDL_Renderer *pRenderer);
void drawMap(SDL_Renderer *pRenderer, SDL_Texture *bgTexture);
void createMaze(SDL_Renderer *pRenderer, SDL_Texture *wallTexture);
void destroyTexture(SDL_Texture *texture);

#endif