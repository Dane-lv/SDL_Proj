#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include "../include/constants.h"
#include "../include/setup_game.h"
#include "../include/process_input.h"
#include "../include/update_game.h"
#include "../include/render_game.h"
#include "../include/game_state.h"


bool initSDL(SDL_App *app) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return false;
    }

    app->window = SDL_CreateWindow("Project",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   WINDOW_WIDTH,
                                   WINDOW_HEIGHT,
                                   0);
    if (app->window == NULL) {
        printf("Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    app->renderer = SDL_CreateRenderer(app->window, -1, 0);
    if (app->renderer == NULL) {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(app->window);
        SDL_Quit();
        return false;
    }

    return true;
}

int main(int argc, char *argv[]){
    (void)argc; //silence the warning
    (void)argv;
    
    SDL_App app;

    if (!initSDL(&app)) {
        return 1;
    }

    app.isRunning = true;

    setupGame(&app);
    while(app.isRunning) {
        processInput(&app);
        updateGame();
        renderGame(&app);
        SDL_Delay(1000/60);
    }

    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();

    return 0;
}
