#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>


typedef struct player Player;

Player *createPlayer(int x, int y, SDL_Renderer *pRenderer);
void drawPlayer(Player *pPlayer);
void updatePlayer(Player *pPlayer, float deltaTime);
void destroyPlayer(Player *pPlayer);
void movePlayerLeft(Player *pPlayer);
void movePlayerRight(Player *pPlayer);
void movePlayerUp(Player *pPlayer);
void movePlayerDown(Player *pPlayer);
void stopMovementVY(Player *pPlayer);
void stopMovementVX(Player *pPlayer);




#endif 