#include <stdio.h>
#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>
#include "../include/constants.h"
#include "../include/game_core.h"
#include "../include/network.h"

#define DEFAULT_PORT 7777

int main(int argc, char** argv)
{
    (void)argc; // Unused parameter
    (void)argv; // Unused parameter
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("SDL Initialization Error: %s\n", SDL_GetError());
        return 1;
    }
    
    // Initialize SDL_net
    if (!netInit()) {
        printf("Failed to initialize SDL_net: %s\n", SDLNet_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Create game context
    NetMgr nm = {0};
    GameContext game = {0};
    
    // Set up context
    game.isHost = true;
    game.isNetworked = true;
    game.netMgr = nm;
    game.netMgr.userData = &game;
    
    // Create window and renderer
    game.window = SDL_CreateWindow("Maze Mayhem - SERVER", 
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!game.window) {
        printf("Window Creation Error: %s\n", SDL_GetError());
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    game.renderer = SDL_CreateRenderer(game.window, -1, 
                                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!game.renderer) {
        printf("Renderer Creation Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(game.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    // Start host on default port
    if (!hostStart(&game.netMgr, DEFAULT_PORT)) {
        printf("Failed to start host: %s\n", SDLNet_GetError());
        SDL_DestroyRenderer(game.renderer);
        SDL_DestroyWindow(game.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    // Initialize game
    if (!gameInit(&game)) {
        printf("Failed to initialize game\n");
        SDL_DestroyRenderer(game.renderer);
        SDL_DestroyWindow(game.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    // Main game loop
    while (game.isRunning) {
        gameCoreRunFrame(&game);
    }
    
    // Cleanup
    gameCoreShutdown(&game);
    
    // Clean up network resources
    if (game.netMgr.server) {
        // Close all client connections
        for (int i = 0; i < game.netMgr.peerCount; i++) {
            if (game.netMgr.peers[i]) {
                SDLNet_TCP_Close(game.netMgr.peers[i]);
            }
        }
        SDLNet_TCP_Close(game.netMgr.server);
    }
    
    if (game.netMgr.set) {
        SDLNet_FreeSocketSet(game.netMgr.set);
    }
    
    // Clean up renderer and window
    if (game.renderer) {
        SDL_DestroyRenderer(game.renderer);
    }
    
    if (game.window) {
        SDL_DestroyWindow(game.window);
    }
    
    netShutdown();
    SDL_Quit();
    
    return 0;
} 