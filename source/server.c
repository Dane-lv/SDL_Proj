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

#define SERVER_PORT 2000

struct server{
    Buffer players[MAX_PLAYERS];
}; typedef struct server Server;

struct dataToClient{
    //
}; typedef struct dataToClient Data;

struct dataFromClient{
    int connection_state;
    IPaddress address;
    Player *player;
    SDL_Event event;
}; typedef struct dataFromClient Buffer;

int main(){
    UDP_Connection conn;
    IPaddress client_ip;
    Data *data;
    Buffer *buffer;
    int received_len;
    int nrOfPlayers = 0;
    Server *server;

    SDLNet_init();

    if(init_udp(&conn, SERVER_PORT) != 0){
        fprintf(stderr, "Failed to initialize UDP connection\n");
        return -1;
    }

    printf("Server is listening on port %d...\n", SERVER_PORT);

    while(1){
        received_len = receive_udp(&conn, &buffer, sizeof(buffer), &client_ip);
        if(received_len > 0){
            printf("Received %d bytes from %s:%d\n", received_len, SDLNet_ResolveIP(&client_ip),
                client_ip.port);

            if(buffer->connection_state == 0 && nrOfPlayers <= MAX_PLAYERS){
                nrOfPlayers++;
                buffer->connection_state = nrOfPlayers;
                buffer->address = client_ip;
            }

            server->players[buffer->connection_state] = *buffer;
            
            send_udp(&conn, &data, sizeof(data), &client_ip);
        }
    }

    close_udp(&conn);
    SDL_Quit();

    return 0;
}





struct game{
    bool isRunning;
    Player *pPlayer;
    Maze *pMaze;
    int tileMap[TILE_WIDTH][TILE_HEIGHT];
    Projectile *pProjectile[MAX_PROJECTILES];
}; typedef struct game Game;