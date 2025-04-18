#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include "../include/constants.h"
#include "../include/player.h"
#include "../include/camera.h"
#include "../include/maze.h"
#include "../include/projectile.h"

#define NAWID

struct game {
    bool isRunning;
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Player *pPlayer;
    Camera *pCamera;
    Maze *pMaze;
    SDL_Texture* tileMapTexture;
    SDL_Surface* tileMapSurface;
    int tileMap[TILE_WIDTH][TILE_HEIGHT];
    Projectile *pProjectile[MAX_PROJECTILES];
};
typedef struct game Game;

bool initiateGame(Game *pGame);
void closeGame(Game *pGame);
void run(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void updateGame(Game *pGame, float deltaTime);
void renderGame(Game *pGame);
void drawCheckerboard(Game *pGame);
void updatePlayerRotation(Game *pGame);

int main(int argc, char** argv)
{
    (void)argc; //silence the warnings
    (void)argv;
    Game g = {0};
    if(!initiateGame(&g)) return 1;
    run(&g);
    closeGame(&g);
    return 0;
}

bool initiateGame(Game *pGame)
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("SDL Initialization Error: %s\n", SDL_GetError());
        return false;
    }
    pGame->pWindow = SDL_CreateWindow("Maze Mayhem", SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
    if (!pGame->pWindow)
    {
        printf("Window Creation Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return false;
    }
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, 0);
    if (!pGame->pRenderer)
    {
        printf("Renderer Creation Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return false;    
    }

    pGame->pPlayer = createPlayer(pGame->pRenderer);

    if(!pGame->pPlayer){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return false;
    }
    pGame->pMaze = createMaze(pGame->pRenderer, pGame->tileMapTexture, pGame->tileMapSurface);
    if(!pGame->pMaze){
        printf("Maze Creation Error: %s\n", SDL_GetError());
        closeGame(pGame);
        return false;
    }
    initiateMap(pGame->pMaze);
    generateMazeLayout(pGame->pMaze);
    pGame->pCamera = createCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!pGame->pCamera){
        printf("Error: Failed to create camera\n");
        closeGame(pGame);
        return false;
    }

    // Initialize projectiles
    for(int i = 0; i < MAX_PROJECTILES; i++){
        pGame->pProjectile[i] = createProjectile(pGame->pRenderer);
        if(!pGame->pProjectile[i]){
            printf("Error: Failed to create projectile %d\n", i);
            closeGame(pGame);
            return false;
        }
    }

    return true;
}

void run(Game *pGame)
{
    pGame->isRunning = true;
    SDL_Event event;

    Uint32 lastTime = SDL_GetTicks(); //time of the first frame since SDl init
    Uint32 currentTime; //time when the current frame started
    float deltaTime; //move player by pixels in second instead of by frames in second

    while(pGame->isRunning)
    {
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; //convert ms to seconds
        lastTime = currentTime;
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                pGame->isRunning = false;
            }
            else
            {
                handleInput(pGame, &event);
            }
        }
        updateGame(pGame, deltaTime);
        updatePlayerRotation(pGame);
        renderGame(pGame);
    }
}

void handleInput(Game *pGame, SDL_Event *pEvent)
{
    if(pEvent->type == SDL_KEYDOWN)
    {
        switch(pEvent->key.keysym.scancode)
        {
            case SDL_SCANCODE_ESCAPE:
                pGame->isRunning = false;
                break;
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                movePlayerUp(pGame->pPlayer);
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                movePlayerDown(pGame->pPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                movePlayerLeft(pGame->pPlayer);
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                movePlayerRight(pGame->pPlayer);
                break;
            case SDL_SCANCODE_SPACE:
                spawnProjectile(pGame->pProjectile, pGame->pPlayer);
                break;
            default:
                break;
        }
    }
    else if(pEvent->type == SDL_KEYUP)
    {
        // Get current WASD key states
        const Uint8 *keyState = SDL_GetKeyboardState(NULL);
        
        switch (pEvent->key.keysym.scancode)
        {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                // If S/DOWN is pressed, immediately start moving down
                if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
                    movePlayerDown(pGame->pPlayer);
                else
                    stopMovementVY(pGame->pPlayer);
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                // If W/UP is pressed, immediately start moving up
                if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP])
                    movePlayerUp(pGame->pPlayer);
                else
                    stopMovementVY(pGame->pPlayer);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                // If D/RIGHT is pressed, immediately start moving right
                if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT])
                    movePlayerRight(pGame->pPlayer);
                else
                    stopMovementVX(pGame->pPlayer);
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                // If A/LEFT is pressed, immediately start moving left
                if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT])
                    movePlayerLeft(pGame->pPlayer);
                else
                    stopMovementVX(pGame->pPlayer);
                break;
            default:
                break;
        }
    }
}

void updateGame(Game *pGame, float deltaTime)
{
    updatePlayer(pGame->pPlayer, deltaTime);
    
    SDL_Rect playerRect = getPlayerRect(pGame->pPlayer);
    bool collision = checkCollision(pGame->pMaze, playerRect);
    if (collision) {
        revertToPreviousPosition(pGame->pPlayer);
    }
    
    // Use the new function that includes wall collision detection
    updateProjectileWithWallCollision(pGame->pProjectile, pGame->pMaze, deltaTime);
    
    updateCamera(pGame->pCamera, pGame->pPlayer);
}

void updatePlayerRotation(Game *pGame)
{
    // Get mouse position in screen coordinates
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Rect playerPos = getPlayerPosition(pGame->pPlayer);
    SDL_Rect screenPlayerPos = getWorldCoordinatesFromCamera(pGame->pCamera, playerPos);
    float playerCenterX = screenPlayerPos.x + screenPlayerPos.w / 2.0f;
    float playerCenterY = screenPlayerPos.y + screenPlayerPos.h / 2.0f;
    
    float deltaX = mouseX - playerCenterX;
    float deltaY = mouseY - playerCenterY;
    float radians = atan2f(deltaY, deltaX);
    float angle = radians * 180.0f / 3.14;
    setPlayerAngle(pGame->pPlayer, angle);
}

void renderGame(Game *pGame)
{
    // Clear with a dark background
    SDL_SetRenderDrawColor(pGame->pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pGame->pRenderer);

    drawMap(pGame->pMaze, pGame->pCamera);
    drawPlayer(pGame->pPlayer, pGame->pCamera);
    drawProjectile(pGame->pProjectile, pGame->pCamera);
    
    SDL_RenderPresent(pGame->pRenderer);
}

void closeGame(Game *pGame)
{
    destroyProjectile(pGame->pProjectile);
    if(pGame->pPlayer) 
        destroyPlayer(pGame->pPlayer);
    if(pGame->pCamera)
        destroyCamera(pGame->pCamera);
    if(pGame->pMaze)
        destroyMaze(pGame->pMaze);
    if(pGame->pRenderer)
        SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow)
        SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}
