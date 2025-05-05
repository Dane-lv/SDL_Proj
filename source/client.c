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
    // --- Argument Handling ---
    char* joinIP;
    if (argc > 1) {
        // Use the first argument as IP if provided
        joinIP = argv[1];
    } else {
        // Otherwise, default to localhost
        joinIP = DEFAULT_IP;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL Initialization Error: %s\n", SDL_GetError());
        return 1;
    }
    
    if (!netInit()) {
        printf("Failed to initialize SDL_net: %s\n", SDLNet_GetError());
        SDL_Quit();
        return 1;
    }
    
    NetMgr nm = {0};
    GameContext ctx = {0};
    
 
    ctx.isHost = false;
    ctx.isNetworked = true;
    ctx.netMgr = nm;
    ctx.netMgr.userData = &ctx;
    
 
    ctx.window = SDL_CreateWindow("Maze Mayhem - CLIENT", 
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!ctx.window) {
        printf("Error: %s\n", SDL_GetError());
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    ctx.renderer = SDL_CreateRenderer(ctx.window, -1, 
                                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!ctx.renderer) {
        printf(" Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
    if (!clientConnect(&ctx.netMgr, joinIP, DEFAULT_PORT)) {
        printf("Failed to connect to server: %s\n", SDLNet_GetError());
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
   
    if (!gameInit(&ctx)) {
        printf("Failed to initialize game\n");
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        SDL_Quit();
        return 1;
    }
    
   
    while (ctx.isRunning) {
        gameCoreRunFrame(&ctx);
    }
    
  
    gameCoreShutdown(&ctx);
    
  
    if (ctx.netMgr.client) {
        SDLNet_TCP_Close(ctx.netMgr.client);
    }
    
    if (ctx.netMgr.set) {
        SDLNet_FreeSocketSet(ctx.netMgr.set);
    }
    
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