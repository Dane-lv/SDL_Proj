#ifndef NETWORK_H
#define NETWORK_H

#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>

#define MAX_PLAYERS 5      // total slots
#define BUF_SIZE    1024   

// Message types
enum {
    MSG_JOIN = 1,      // client → host: wants to join
    MSG_POS,           // client → host: player position update
    MSG_SHOOT,         // client → host: projectile spawn
    MSG_STATE,         // host  → all: full game state
    MSG_LEAVE          // client/host: disconnecting
};

// Message header structure (4 bytes)
typedef struct {
    Uint8 type;        // Message type (MSG_*)
    Uint8 playerId;    // Player ID (0-4)
    Uint16 size;       // Size of payload
} MessageHeader;

typedef struct {
    TCPsocket      server;               // host listens here
    TCPsocket      client;               // client connects here
    TCPsocket      peers[MAX_PLAYERS];   // host's array of client sockets
    int            peerCount;
    SDLNet_SocketSet set;
    char           buf[BUF_SIZE];        // single fixed buffer
    bool           isHost;
    Uint8          localPlayerId;        // ID of local player (assigned by host)
    void*          userData;             // Pointer to game context
} NetMgr;

bool  netInit(void);
void  netShutdown(void);

// Host functions
bool  hostStart(NetMgr *nm, int port);
void  hostTick(NetMgr *nm, void *ctx);

// Client functions
bool  clientConnect(NetMgr *nm, const char *ip, int port);
void  clientTick(NetMgr *nm, void *ctx);

// Send functions
bool  sendPlayerPosition(NetMgr *nm, float x, float y, float angle);
bool  sendPlayerShoot(NetMgr *nm, float x, float y, float angle);

#endif // NETWORK_H 