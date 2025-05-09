#ifndef GAME_CORE_H
#define GAME_CORE_H

#include <SDL.h>
#include <stdbool.h>
#include "player.h"
#include "camera.h"
#include "maze.h"
#include "projectile.h"
#include "network.h"
#include "constants.h"

// Game context structure to replace globals
typedef struct {
    bool isRunning;
    SDL_Window *window;
    SDL_Renderer *renderer;
    Player *localPlayer;
    Player *players[MAX_PLAYERS];
    Camera *camera;
    Maze *maze;
    Projectile *projectiles[MAX_PROJECTILES];
    NetMgr netMgr;
    bool isHost;
    bool isNetworked;
    int frameCounter;
    
    // New fields for death screen and spectate mode
    bool isSpectating;
    bool showDeathScreen;
    SDL_Texture *fontTexture;
    SDL_Rect spectateButtonRect;
} GameContext;

// Core game functions
bool gameInit(GameContext *ctx);
void gameCoreRunFrame(GameContext *ctx);
void gameCoreShutdown(GameContext *ctx);
void handleInput(GameContext *ctx, SDL_Event *event);
void updateGame(GameContext *ctx, float deltaTime);
void updatePlayerRotation(GameContext *ctx);
void renderGame(GameContext *ctx);

// New functions for death screen and spectate mode
void checkPlayerProjectileCollisions(GameContext *ctx);
void renderDeathScreen(GameContext *ctx);
void enableSpectateMode(GameContext *ctx);

// Network event handling
void gameOnNetworkMessage(GameContext *ctx, Uint8 type, Uint8 playerId, const void *data, int size);

#endif // GAME_CORE_H 