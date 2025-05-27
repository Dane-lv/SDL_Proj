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
#include "audio_manager.h"

typedef struct
{
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

    bool lobbyOpen;
    bool lobbyReady;
    bool isSpectating;
    bool showDeathScreen;
    bool lobbyReceivedStart;
    SDL_Texture *fontTexture;
    SDL_Rect spectateButtonRect;
    AudioManager *audioManager;
} GameContext;

bool gameInit(GameContext *);
void gameCoreRunFrame(GameContext *);
void gameCoreShutdown(GameContext *);

void handleInput(GameContext *, SDL_Event *);
void updateGame(GameContext *, float dt);
void updatePlayerRotation(GameContext *);
void renderGame(GameContext *);

void gameOnNetworkMessage(GameContext *, Uint8 type, Uint8 pid,
                          const void *data, int size);

#endif