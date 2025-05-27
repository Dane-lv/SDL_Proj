#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>
#include "camera.h"
#include "constants.h"

typedef struct player Player;

Player *createPlayer(SDL_Renderer *pRenderer);
void drawPlayer(Player *pPlayer, Camera *pCamera);
void updatePlayer(Player *pPlayer, float deltaTime);
void destroyPlayer(Player *pPlayer);
void movePlayerLeft(Player *pPlayer);
void movePlayerRight(Player *pPlayer);
void movePlayerUp(Player *pPlayer);
void movePlayerDown(Player *pPlayer);
void stopMovementVY(Player *pPlayer);
void stopMovementVX(Player *pPlayer);

SDL_Rect getPlayerPosition(Player *pPlayer);
SDL_Rect getPlayerRect(Player *pPlayer);
void setPlayerPosition(Player *pPlayer, float x, float y);
void setPlayerAngle(Player *pPlayer, float angle);
float getPlayerAngle(Player *pPlayer);
void revertToPreviousPosition(Player *pPlayer);

void playerSetTextureById(Player *pPlayer, SDL_Renderer *pRenderer, int playerId);

bool isPlayerAlive(Player *pPlayer);
void killPlayer(Player *pPlayer);

#endif