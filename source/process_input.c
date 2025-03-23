#include "../include/process_input.h"
#include <SDL.h>

void processInput(SDL_App *app) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            app->isRunning = false;
        }
    }
}
