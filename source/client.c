#include <stdio.h>
#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>
#include <string.h>
#include "../include/constants.h"
#include "../include/game_core.h"
#include "../include/network.h"

#define DEFAULT_PORT 7777
#define DEFAULT_IP "127.0.0.1"

int main(int argc, char** argv)
{
    char* joinIP;
    if (argc > 1) {
        joinIP = argv[1];
    } else {
        joinIP = DEFAULT_IP;
    }


    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Fel: Kunde inte initiera SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (!netInit()) {
        printf("Fel: Kunde inte initiera SDL_net: %s\n", SDLNet_GetError());
        SDL_Quit();
        return 1;
    }

    GameContext game = {0};
    game.isHost = false;
    game.isNetworked = true;

    game.window = SDL_CreateWindow("Maze Mayhem - CLIENT",
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!game.window) {
        printf("Fel: Kunde inte skapa fönster: %s\n", SDL_GetError());
        netShutdown();
        SDL_Quit();
        return 1;
    }

    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    if (!game.renderer) {
        printf("Fel: Kunde inte skapa renderare: %s\n", SDL_GetError());
        SDL_DestroyWindow(game.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }

    printf("Ansluter till server %s:%d...\n", joinIP, DEFAULT_PORT);
    if (!clientConnect(&game.netMgr, joinIP, DEFAULT_PORT)) {
        printf("Fel: Kunde inte ansluta till servern.\n");
        SDL_DestroyRenderer(game.renderer);
        SDL_DestroyWindow(game.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    printf("Ansluten!\n");

    if (!gameInit(&game)) {
        printf("Fel: Kunde inte initiera spelet.\n");
        if (game.netMgr.client) SDLNet_TCP_Close(game.netMgr.client);
        if (game.netMgr.set) SDLNet_FreeSocketSet(game.netMgr.set);
        SDL_DestroyRenderer(game.renderer);
        SDL_DestroyWindow(game.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }

    // --- Main Game Loop ---
    while (game.isRunning) {
        gameCoreRunFrame(&game);
    }

   
    gameCoreShutdown(&game); 

    if (game.netMgr.client) SDLNet_TCP_Close(game.netMgr.client);
    if (game.netMgr.set) SDLNet_FreeSocketSet(game.netMgr.set);

    if (game.renderer) SDL_DestroyRenderer(game.renderer);
    if (game.window) SDL_DestroyWindow(game.window);

    netShutdown();
    SDL_Quit();

    return 0;
} 