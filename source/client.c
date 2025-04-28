#include <stdio.h>
#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>

#include "../include/constants.h"
#include "../include/player.h"
#include "../include/camera.h"
#include "../include/maze.h"
#include "../include/projectile.h"
#include "../include/net_utils.h"

struct client_to_server{
    //
}; typedef DataToServer;

void main(){
    UDP_Connection conn;
    if(init_udp(&conn, 0) != 0){
        // Hantera fel
    }

    IPaddress client_addr;
    char buffer[512];

    if(receive_udp(&conn, buffer, sizeof(buffer), &client_addr)){
        printf("Mottaget: %s\n", buffer);
        send_udp(&conn, "Svar från server", 10, &client_addr);
    }
}

struct game{
    bool isRunning;
    Player *pPlayer;
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Camera *pCamera;
    SDL_Texture* tileMapTexture;
    SDL_Surface* tileMapSurface;
}; typedef struct game Game;