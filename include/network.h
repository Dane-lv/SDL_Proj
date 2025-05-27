#ifndef NETWORK_H
#define NETWORK_H

#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>
#include "constants.h"
enum
{
    MSG_JOIN = 1,
    MSG_POS,
    MSG_SHOOT,
    MSG_STATE,
    MSG_LEAVE,
    MSG_DEATH,
    MSG_START
};

#define BUF_SIZE 1024

typedef struct
{
    Uint8 type;
    Uint8 playerId;
    Uint16 size;
} MessageHeader;

typedef struct NetMgr
{
    TCPsocket server;
    TCPsocket client;
    TCPsocket peers[MAX_PLAYERS];
    int peerCount;
    SDLNet_SocketSet set;
    char buf[BUF_SIZE];
    bool isHost;
    Uint8 localPlayerId;
    void *userData;
} NetMgr;

bool netInit(void);
void netShutdown(void);

bool hostStart(NetMgr *nm, int port);
void hostTick(NetMgr *nm, void *game);

bool clientConnect(NetMgr *nm, const char *ip, int port);
void clientTick(NetMgr *nm, void *game);

bool sendPlayerPosition(NetMgr *nm, float x, float y, float angle);
bool sendPlayerShoot(NetMgr *nm, float x, float y, float angle, int pid);
bool sendPlayerDeath(NetMgr *nm, Uint8 killerId);
bool sendStartGame(NetMgr *nm);

#endif