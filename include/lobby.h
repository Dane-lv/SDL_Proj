#ifndef LOBBY_H
#define LOBBY_H

#include <SDL.h>
#include <stdbool.h>
#include "game_core.h"

typedef struct lobby Lobby;

Lobby *lobbyCreate(SDL_Renderer *r, SDL_Window *w,
                   GameContext *ctx, bool isHost);
void lobbyDestroy(Lobby *l);

bool lobbyHandleEvent(Lobby *l, const SDL_Event *e);
void lobbyRender(Lobby *l);

bool lobbyIsReady(const Lobby *l);
bool lobbyBackPressed(const Lobby *l);
#endif