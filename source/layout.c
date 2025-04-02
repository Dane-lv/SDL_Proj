#include "../include/constants.h"
#include "../include/layout.h"
#include <SDL.h>
#include <SDL_image.h>

SDL_Texture* initiateMap(SDL_Renderer *pRenderer)
{
    SDL_Surface *pSurface=IMG_Load("resources/Bakgrund.jpg");
    SDL_Texture *bgTexture=SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    return bgTexture;
}

SDL_Texture* initiateMaze(SDL_Renderer *pRenderer)
{
    SDL_Surface *pSurface=IMG_Load("resources/WallTexture.png");
    SDL_Texture *wallTexture=SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    return wallTexture;
}

void drawMap(SDL_Renderer *pRenderer, SDL_Texture *bgTexture)
{
    SDL_Rect bgRect={0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(pRenderer, bgTexture, NULL, &bgRect);
}

void createMaze(SDL_Renderer *pRenderer, SDL_Texture *wallTexture)
{
    SDL_SetRenderDrawColor(pRenderer, 255,255,255,255);
    SDL_Rect walls[] = {
        {750,  50, 15, 500},    //Yttre höger vertikal vägg
        { 30,  50, 15, 500},    //Yttre vänster vertikal vägg
        { 45,  50, 705, 15},    //Yttre  övre horisontell vägg
        { 45, 535, 705, 15},    //Yttre nedre horisontell vägg
        { 90, 300, 200, 10},    //horisontell vägg från yttre vänster vägg
        {290, 250,  10, 150},   //Vertikal vägg slutet av förra
        {200, 400,  10, 150},   //Vertikal vägg från yttre nedre vägg
        {290, 400, 250, 10},    //Horisontell vägg slutet av för-förra
        {540, 325,  10, 125},   //Vertikal vägg slutet av förra
        {425, 220, 275, 10},    //Horisontell vägg från yttre höger vägg
        {415, 150,  10, 150},   //Vertikal vägg slutet av förra
        {185, 150, 400, 10},    //Horisontell vägg slutet av förra
        {185, 150,  10, 85}     //Vertikal vägg slutet av förra
    };
    int numWalls = sizeof(walls) / sizeof(walls[0]);
    for(int i = 0; i < numWalls; i++)
    {
        SDL_RenderCopy(pRenderer, wallTexture, NULL, &walls[i]);
    }
}

void destroyMap(SDL_Texture *texture)
{
    SDL_DestroyTexture(texture);
    texture=NULL;
}

