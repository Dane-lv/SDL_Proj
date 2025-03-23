#include "../include/render_game.h"

void renderGame(SDL_App *app) {
    
    SDL_SetRenderDrawColor(app->renderer, 255, 0, 0, 255);
    SDL_RenderClear(app->renderer);
    SDL_RenderPresent(app->renderer);
}
