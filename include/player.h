#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>


typedef struct player Player;

Player *createPlayer(int x, int y, SDL_Renderer *pRenderer);
void drawPlayer(Player *pPlayer);



#endif 