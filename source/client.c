#include <stdio.h>
#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>
#include <string.h>
#include "../include/constants.h"
#include "../include/game_core.h"
#include "../include/network.h"

#define DEFAULT_PORT 7777

int main(int argc, char** argv)
{
    char* joinIP = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--join") == 0 && i + 1 < argc) {
            joinIP = argv[i + 1];
            i++; // Skip next argument (IP address)
        }
    }
    
    // If no IP was specified, use localhost
    if (joinIP == NULL) {
        joinIP = "127.0.0.1";
        printf("No IP specified, connecting to localhost\n");
    }
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
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
    GameContext ctx = {0};
    
    // Set up context
    ctx.isHost = false;
    ctx.isNetworked = true;
    ctx.netMgr = nm;
    ctx.netMgr.userData = &ctx;
    
    // Create window and renderer
    ctx.window = SDL_CreateWindow("Maze Mayhem - CLIENT", 
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!ctx.window) {
        printf("Window Creation Error: %s\n", SDL_GetError());
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    ctx.renderer = SDL_CreateRenderer(ctx.window, -1, 
                                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!ctx.renderer) {
        printf("Renderer Creation Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    // Connect to server
    if (!clientConnect(&ctx.netMgr, joinIP, DEFAULT_PORT)) {
        printf("Failed to connect to server: %s\n", SDLNet_GetError());
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    // Initialize game
    if (!gameInit(&ctx)) {
        printf("Failed to initialize game\n");
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    // Main game loop
    while (ctx.isRunning) {
        gameCoreRunFrame(&ctx);
    }
    
    // Cleanup
    gameCoreShutdown(&ctx);
    
    // Clean up network resources
    if (ctx.netMgr.client) {
        SDLNet_TCP_Close(ctx.netMgr.client);
    }
    
    if (ctx.netMgr.set) {
        SDLNet_FreeSocketSet(ctx.netMgr.set);
    }
    
    // Clean up renderer and window
    if (ctx.renderer) {
        SDL_DestroyRenderer(ctx.renderer);
    }
    
    if (ctx.window) {
        SDL_DestroyWindow(ctx.window);
    }
    
    netShutdown();
    SDL_Quit();
    
    return 0;
} 