#ifndef LOBBY_H
#define LOBBY_H

#include <SDL.h>
#include <stdbool.h>
#include "game_core.h"

typedef struct lobby Lobby;

/* isHost → visar även START-knapp */
Lobby *lobbyCreate (SDL_Renderer *r, SDL_Window *w,
                    GameContext *ctx, bool isHost);
void   lobbyDestroy(Lobby *l);

bool   lobbyHandleEvent (Lobby *l, const SDL_Event *e);
void   lobbyRender      (Lobby *l);

bool   lobbyIsReady     (const Lobby *l);   /* host har tryckt START   */
bool   lobbyBackPressed (const Lobby *l);   /* någon har tryckt BACK   */

#endif /* LOBBY_H */