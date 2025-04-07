#include "../include/constants.h"
#include "../include/layout.h"
#include "../include/maze.h"

void mazeLayout1(Maze* pMaze)
{
    clearWalls(pMaze);
    
    int wallThickness = 8;
    
    // Border walls
    addWall(pMaze, 0, 0, WINDOW_WIDTH, wallThickness);                // Top border
    addWall(pMaze, 0, WINDOW_HEIGHT - wallThickness, WINDOW_WIDTH, wallThickness); // Bottom border
    addWall(pMaze, 0, 0, wallThickness, WINDOW_HEIGHT);                // Left border
    addWall(pMaze, WINDOW_WIDTH - wallThickness, 0, wallThickness, WINDOW_HEIGHT); // Right border

    // Inner walls - using the same layout that was in createMaze
    addWall(pMaze, 750,  50, 15, 500);    //Yttre höger vertikal vägg
    addWall(pMaze, 30,  50, 15, 500);    //Yttre vänster vertikal vägg
    addWall(pMaze, 45,  50, 705, 15);    //Yttre övre horisontell vägg
    addWall(pMaze, 45, 535, 705, 15);    //Yttre nedre horisontell vägg
    addWall(pMaze, 90, 300, 200, 10);    //horisontell vägg från yttre vänster vägg
    addWall(pMaze, 290, 250,  10, 150);   //Vertikal vägg slutet av förra
    addWall(pMaze, 200, 400,  10, 150);   //Vertikal vägg från yttre nedre vägg
    addWall(pMaze, 290, 400, 250, 10);    //Horisontell vägg slutet av för-förra
    addWall(pMaze, 540, 325,  10, 125);   //Vertikal vägg slutet av förra
    addWall(pMaze, 425, 220, 275, 10);    //Horisontell vägg från yttre höger vägg
    addWall(pMaze, 415, 150,  10, 150);   //Vertikal vägg slutet av förra
    addWall(pMaze, 185, 150, 400, 10);    //Horisontell vägg slutet av förra
    addWall(pMaze, 185, 150,  10, 85);    //Vertikal vägg slutet av förra
}

