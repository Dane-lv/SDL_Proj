#include <stdio.h>
#include <SDL.h>
#include <SDL_net.h>
#include <SDL_ttf.h>             
#include <stdbool.h>
#include <string.h>
#include "../include/constants.h"
#include "../include/game_core.h"
#include "../include/network.h"
#include "../include/menu.h"        

#define DEFAULT_PORT 7777

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* -------------------------------------------------- */
    /* Init‑block                                         */
    /* -------------------------------------------------- */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL Initialization Error: %s\n", SDL_GetError());
        return 1;
    }

    /* --- NEW --- SDL_ttf */
    if (TTF_Init() != 0) {
        printf("TTF Initialization Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (!netInit()) {
        printf("Failed to initialize SDL_net: %s\n", SDLNet_GetError());
        TTF_Quit();                /* --- NEW --- */
        SDL_Quit();
        return 1;
    }

    /* -------------------------------------------------- */
    /* Skapa fönster & renderer (behövs för menyn)        */
    /* -------------------------------------------------- */
    NetMgr      nm  = {0};
    GameContext ctx = {0};

    ctx.window = SDL_CreateWindow("Maze Mayhem",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!ctx.window) {
        printf("Window Creation Error: %s\n", SDL_GetError());
        netShutdown();
        TTF_Quit();                /* --- NEW --- */
        SDL_Quit();
        return 1;
    }

    ctx.renderer = SDL_CreateRenderer(ctx.window, -1,
                         SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!ctx.renderer) {
        printf("Renderer Creation Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        TTF_Quit();                /* --- NEW --- */
        SDL_Quit();
        return 1;
    }

    /* -------------------------------------------------- */
    /* --- NEW --- huvudmeny                              */
    /* -------------------------------------------------- */
    Menu *menu = menuCreate(ctx.renderer, ctx.window);

    bool menuRunning = true;
    while (menuRunning) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            menuRunning = menuHandleEvent(menu, &ev);
        }
        menuRender(menu);

        if (menuGetChoice(menu) != MENU_CHOICE_NONE)
            break;

        SDL_Delay(16); /* ~60 fps */
    }

    MenuChoice choice = menuGetChoice(menu);
    if (choice == MENU_CHOICE_QUIT || choice == MENU_CHOICE_NONE) {
        menuDestroy(menu);
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        TTF_Quit();                /* --- NEW --- */
        SDL_Quit();
        return 0;
    }

    char joinIpBuf[64] = "127.0.0.1";
    bool startAsHost   = false;

    if (choice == MENU_CHOICE_HOST) {
        startAsHost = true;
    } else if (choice == MENU_CHOICE_JOIN) {
        strncpy(joinIpBuf, menuGetJoinIP(menu), sizeof(joinIpBuf) - 1);
        joinIpBuf[sizeof(joinIpBuf) - 1] = '\0';
    }

    menuDestroy(menu);
    /* -------------------------------------------------- */

    /* -------------------------------------------------- */
    /* Setup GameContext & nätverk                        */
    /* -------------------------------------------------- */
    ctx.isHost       = startAsHost;          /* --- NEW --- */
    ctx.isNetworked  = true;
    ctx.netMgr       = nm;
    ctx.netMgr.userData = &ctx;

    bool netOk = false;
    if (startAsHost) {
        netOk = hostStart(&ctx.netMgr, DEFAULT_PORT);   /* --- NEW --- */
    } else {
        netOk = clientConnect(&ctx.netMgr, joinIpBuf, DEFAULT_PORT); /* --- NEW --- */
    }

    if (!netOk) {
        printf("Failed to %s: %s\n",
               startAsHost ? "start host" : "connect to server",
               SDLNet_GetError());

        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        TTF_Quit();            /* --- NEW --- */
        SDL_Quit();
        return 1;
    }

    /* -------------------------------------------------- */
    /* Init spelet                                        */
    /* -------------------------------------------------- */
    if (!gameInit(&ctx)) {
        printf("Failed to initialize game\n");
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        netShutdown();
        TTF_Quit();            /* --- NEW --- */
        SDL_Quit();
        return 1;
    }

    /* -------------------------------------------------- */
    /* Huvud‑loop                                         */
    /* -------------------------------------------------- */
    while (ctx.isRunning) {
        gameCoreRunFrame(&ctx);
    }

    /* -------------------------------------------------- */
    /* Städning                                           */
    /* -------------------------------------------------- */
    gameCoreShutdown(&ctx);

    if (ctx.netMgr.client)
        SDLNet_TCP_Close(ctx.netMgr.client);

    if (ctx.netMgr.server) {
        for (int i = 0; i < ctx.netMgr.peerCount; ++i)
            if (ctx.netMgr.peers[i])
                SDLNet_TCP_Close(ctx.netMgr.peers[i]);
        SDLNet_TCP_Close(ctx.netMgr.server);
    }

    if (ctx.netMgr.set)
        SDLNet_FreeSocketSet(ctx.netMgr.set);

    if (ctx.renderer) SDL_DestroyRenderer(ctx.renderer);
    if (ctx.window)   SDL_DestroyWindow(ctx.window);

    netShutdown();
    TTF_Quit();                        /* --- NEW --- */
    SDL_Quit();
    return 0;
}