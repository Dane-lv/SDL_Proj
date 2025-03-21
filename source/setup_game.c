#include "../include/setup_game.h"
#include <stdio.h>

void setupGame(SDL_App *app) {
    // Mark app as unused to silence the warning
    (void)app;
    printf("Game setup complete\n");
}
