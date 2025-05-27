#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include "../include/player.h"
#include "../include/constants.h"
#include "../include/camera.h"

struct player
{
    float x, y;
    float prevX, prevY;
    float vy, vx;
    float angle;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect playerRect;
    Object_ID objectID;
    bool isAlive;
};

Player *createPlayer(SDL_Renderer *pRenderer)
{
    Player *pPlayer = malloc(sizeof(struct player));
    SDL_Surface *pSurface = IMG_Load("resources/player_1.png");
    if (!pSurface)
    {
        printf("Error loading initial player surface (player_1.png): %s\n", IMG_GetError());
        free(pPlayer);
        return NULL;
    }

    pPlayer->pRenderer = pRenderer;
    pPlayer->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    if (!pPlayer->pTexture)
    {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }

    pPlayer->x = WORLD_WIDTH / 2;
    pPlayer->y = WORLD_HEIGHT / 2;
    pPlayer->prevX = pPlayer->x;
    pPlayer->prevY = pPlayer->y;
    pPlayer->vx = 0;
    pPlayer->vy = 0;
    pPlayer->angle = 0;
    pPlayer->objectID = OBJECT_ID_PLAYER;
    pPlayer->isAlive = true;

    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
    pPlayer->playerRect.w = PLAYERWIDTH;
    pPlayer->playerRect.h = PLAYERHEIGHT;

    return pPlayer;
}

void drawPlayer(Player *pPlayer, Camera *pCamera)
{

    if (!pPlayer->isAlive)
        return;

    SDL_Rect playerRect = pPlayer->playerRect;
    SDL_Rect adjustedRect = getWorldCoordinatesFromCamera(pCamera, playerRect);

    SDL_RenderCopyEx(pPlayer->pRenderer, pPlayer->pTexture, NULL, &adjustedRect, pPlayer->angle + 90.0f, NULL, SDL_FLIP_NONE);
}

void updatePlayer(Player *pPlayer, float deltaTime)
{

    if (!pPlayer->isAlive)
        return;

    pPlayer->prevX = pPlayer->x;
    pPlayer->prevY = pPlayer->y;

    if (pPlayer->vx != 0 && pPlayer->vy != 0)
    {

        float normalizedVx = pPlayer->vx * 0.7071f;
        float normalizedVy = pPlayer->vy * 0.7071f;

        pPlayer->x += normalizedVx * deltaTime;
        pPlayer->y += normalizedVy * deltaTime;
    }
    else
    {

        pPlayer->x += pPlayer->vx * deltaTime;
        pPlayer->y += pPlayer->vy * deltaTime;
    }

    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
}

bool isPlayerAlive(Player *pPlayer)
{
    return pPlayer->isAlive;
}

void killPlayer(Player *pPlayer)
{
    pPlayer->isAlive = false;
    pPlayer->vx = 0;
    pPlayer->vy = 0;
}

void movePlayerLeft(Player *pPlayer)
{
    if (!pPlayer->isAlive)
        return;
    pPlayer->vx = -PLAYERSPEED;
}

void movePlayerRight(Player *pPlayer)
{
    if (!pPlayer->isAlive)
        return;
    pPlayer->vx = PLAYERSPEED;
}

void movePlayerUp(Player *pPlayer)
{
    if (!pPlayer->isAlive)
        return;
    pPlayer->vy = -PLAYERSPEED;
}

void movePlayerDown(Player *pPlayer)
{
    if (!pPlayer->isAlive)
        return;
    pPlayer->vy = PLAYERSPEED;
}

void stopMovementVY(Player *pPlayer)
{
    pPlayer->vy = 0;
}

void stopMovementVX(Player *pPlayer)
{
    pPlayer->vx = 0;
}

SDL_Rect getPlayerPosition(Player *pPlayer)
{
    return pPlayer->playerRect;
}

SDL_Rect getPlayerRect(Player *pPlayer)
{
    return pPlayer->playerRect;
}

SDL_Texture *getPlayerTexture(Player *pPlayer)
{
    return pPlayer->pTexture;
}

void setPlayerPosition(Player *pPlayer, float x, float y)
{
    pPlayer->x = x;
    pPlayer->y = y;
    pPlayer->playerRect.x = (int)x;
    pPlayer->playerRect.y = (int)y;
}

void setPlayerAngle(Player *pPlayer, float angle)
{
    pPlayer->angle = angle;
}

float getPlayerAngle(Player *pPlayer)
{
    return pPlayer->angle;
}

void destroyPlayer(Player *pPlayer)
{
    if (pPlayer->pTexture)
    {
        SDL_DestroyTexture(pPlayer->pTexture);
    }
    free(pPlayer);
}

void revertToPreviousPosition(Player *pPlayer)
{
    pPlayer->x = pPlayer->prevX;
    pPlayer->y = pPlayer->prevY;
    pPlayer->playerRect.x = (int)pPlayer->x;
    pPlayer->playerRect.y = (int)pPlayer->y;
}

void playerSetTextureById(Player *pPlayer, SDL_Renderer *pRenderer, int playerId)
{
    if (!pPlayer || !pRenderer)
    {
        return;
    }

    char texturePath[100];
    int textureFileId = playerId + 1;

    if (textureFileId < 1 || textureFileId > MAX_PLAYERS)
    {
        printf("Warning: Ogiltigt textureFileId %d (från playerId %d). Använder nuvarande/initial textur.\n", textureFileId, playerId);

        return;
    }

    sprintf(texturePath, "resources/player_%d.png", textureFileId);

    SDL_Surface *pSurface = IMG_Load(texturePath);
    if (!pSurface)
    {
        printf("Kunde inte ladda texturyta %s: %s\n", texturePath, IMG_GetError());

        return;
    }

    SDL_Texture *newTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);

    if (!newTexture)
    {
        printf("Kunde inte skapa textur från %s: %s\n", texturePath, SDL_GetError());
        return;
    }

    if (pPlayer->pTexture)
    {
        SDL_DestroyTexture(pPlayer->pTexture);
    }
    pPlayer->pTexture = newTexture;
}