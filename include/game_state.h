#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <SDL.h>
#include <stdbool.h>

typedef struct {
    bool isRunning;
    SDL_Window *window;
    SDL_Renderer *renderer;
} SDL_App;

#endif 

