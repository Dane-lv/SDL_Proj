#ifndef GAME_CORE_H
#define GAME_CORE_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "player.h"
#include "camera.h"
#include "maze.h"
#include "projectile.h"
#include "network.h"
#include "constants.h"
#include "text_renderer.h"
#include "audio_manager.h"

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
    TextRenderer *textRenderer;  // Text rendering component
    AudioManager audioManager;   // Audio manager component
} GameContext;

// Core game functions
bool gameInit(GameContext *ctx);
void gameCoreRunFrame(GameContext *ctx);
void gameCoreShutdown(GameContext *ctx);
void handleInput(GameContext *ctx, SDL_Event *event);
void updateGame(GameContext *ctx, float deltaTime);
void updatePlayerRotation(GameContext *ctx);
void renderGame(GameContext *ctx);
void checkProjectilePlayerCollisions(GameContext *ctx);

// Network event handling
void gameOnNetworkMessage(GameContext *ctx, Uint8 type, Uint8 playerId, const void *data, int size);

#endif // GAME_CORE_H 