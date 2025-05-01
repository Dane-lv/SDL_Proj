#include "../include/network.h"
#include <string.h>
#include <stdio.h>

// Function to handle network data (declared here, implemented in main.c)
extern void handleNetworkData(Uint8 type, Uint8 playerId, const void *data, int size);

bool netInit(void) {
    return SDLNet_Init() == 0;
}

void netShutdown(void) {
    SDLNet_Quit();
}

// HOST: open listening socket & socket set
bool hostStart(NetMgr *nm, int port) {
    IPaddress ip;
    
    if (SDLNet_ResolveHost(&ip, NULL, port) < 0) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return false;
    }
    
    // Create socket set for managing connections
    nm->set = SDLNet_AllocSocketSet(MAX_PLAYERS + 1);
    if (!nm->set) {
        printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        return false;
    }
    
    // Create server socket
    nm->server = SDLNet_TCP_Open(&ip);
    if (!nm->server) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        SDLNet_FreeSocketSet(nm->set);
        return false;
    }
    
    // Add server socket to socket set
    SDLNet_TCP_AddSocket(nm->set, nm->server);
    
    // Initialize peer count
    nm->peerCount = 0;
    nm->isHost = true;
    nm->localPlayerId = 0; // Host is always player 0
    
    printf("Host started on port %d\n", port);
    return true;
}

// Called each frame by host
void hostTick(NetMgr *nm) {
    int ready = SDLNet_CheckSockets(nm->set, 0);
    if (ready <= 0) return;

    // 1) Accept new connections
    if (SDLNet_SocketReady(nm->server) && nm->peerCount < MAX_PLAYERS-1) {
        TCPsocket newClient = SDLNet_TCP_Accept(nm->server);
        if (newClient) {
            // Assign a player ID to the new client
            Uint8 newPlayerId = nm->peerCount + 1; // Player IDs start at 1 for clients (0 is host)
            
            // Store the client socket
            nm->peers[nm->peerCount++] = newClient;
            SDLNet_TCP_AddSocket(nm->set, newClient);
            
            // Send back join ack with player ID
            MessageHeader header = {
                .type = MSG_JOIN,
                .playerId = newPlayerId,
                .size = 0
            };
            
            memcpy(nm->buf, &header, sizeof(MessageHeader));
            SDLNet_TCP_Send(newClient, nm->buf, sizeof(MessageHeader));
            
            printf("New client connected, assigned ID: %d\n", newPlayerId);
        }
        ready--;
    }

    // 2) Receive from clients & broadcast
    for (int i = 0; i < nm->peerCount && ready > 0; ++i) {
        TCPsocket s = nm->peers[i];
        if (SDLNet_SocketReady(s)) {
            int len = SDLNet_TCP_Recv(s, nm->buf, BUF_SIZE);
            if (len <= 0) {
                // Client disconnected
                printf("Client %d disconnected\n", i + 1);
                SDLNet_TCP_DelSocket(nm->set, s);
                SDLNet_TCP_Close(s);
                
                // Shift remaining peers to fill the gap
                if (i < nm->peerCount - 1) {
                    nm->peers[i] = nm->peers[nm->peerCount - 1];
                }
                nm->peerCount--;
                i--;
            } else if ((unsigned int)len >= sizeof(MessageHeader)) {
                // Process received data
                MessageHeader *header = (MessageHeader*)nm->buf;
                
                // Process message locally first
                handleNetworkData(header->type, header->playerId, 
                                 nm->buf + sizeof(MessageHeader), 
                                 header->size);
                
                // Broadcast to all other clients
                for (int j = 0; j < nm->peerCount; ++j) {
                    if (j != i) { // Don't send back to originator
                        SDLNet_TCP_Send(nm->peers[j], nm->buf, len);
                    }
                }
            }
            ready--;
        }
    }
}

// CLIENT: connect to host & add to set
bool clientConnect(NetMgr *nm, const char *ip, int port) {
    IPaddress serverIP;
    
    if (SDLNet_ResolveHost(&serverIP, ip, port) < 0) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return false;
    }
    
    // Create client socket
    nm->client = SDLNet_TCP_Open(&serverIP);
    if (!nm->client) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        return false;
    }
    
    // Create socket set
    nm->set = SDLNet_AllocSocketSet(1);
    if (!nm->set) {
        printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        SDLNet_TCP_Close(nm->client);
        return false;
    }
    
    // Add client socket to socket set
    SDLNet_TCP_AddSocket(nm->set, nm->client);
    
    nm->isHost = false;
    nm->localPlayerId = 0xFF; // Invalid ID until assigned by host
    
    printf("Connected to server at %s:%d\n", ip, port);
    return true;
}

// Called each frame by client
void clientTick(NetMgr *nm) {
    int ready = SDLNet_CheckSockets(nm->set, 0);
    if (ready <= 0) return;
    
    if (SDLNet_SocketReady(nm->client)) {
        int len = SDLNet_TCP_Recv(nm->client, nm->buf, BUF_SIZE);
        if (len <= 0) {
            printf("Disconnected from server\n");
            // Handle disconnect - could set a flag to let the game know
            return;
        } else if ((unsigned int)len >= sizeof(MessageHeader)) {
            MessageHeader *header = (MessageHeader*)nm->buf;
            
            // Check if this is a join confirmation
            if (header->type == MSG_JOIN && nm->localPlayerId == 0xFF) {
                nm->localPlayerId = header->playerId;
                printf("Assigned player ID: %d\n", nm->localPlayerId);
            } else {
                // Process received data
                handleNetworkData(header->type, header->playerId,
                                 nm->buf + sizeof(MessageHeader),
                                 header->size);
            }
        }
    }
}

// Send player position to the network
bool sendPlayerPosition(NetMgr *nm, float x, float y, float angle) {
    MessageHeader header = {
        .type = MSG_POS,
        .playerId = nm->localPlayerId,
        .size = sizeof(float) * 3
    };
    
    // Pack header and data
    int offset = 0;
    memcpy(nm->buf + offset, &header, sizeof(MessageHeader));
    offset += sizeof(MessageHeader);
    
    // Pack position data
    memcpy(nm->buf + offset, &x, sizeof(float));
    offset += sizeof(float);
    memcpy(nm->buf + offset, &y, sizeof(float));
    offset += sizeof(float);
    memcpy(nm->buf + offset, &angle, sizeof(float));
    offset += sizeof(float);
    
    // Send the data
    int bytesSent = 0;
    
    if (nm->isHost) {
        // Host sends to all clients
        for (int i = 0; i < nm->peerCount; i++) {
            bytesSent = SDLNet_TCP_Send(nm->peers[i], nm->buf, offset);
            if (bytesSent < offset) {
                printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
                return false;
            }
        }
        // Also process locally
        handleNetworkData(MSG_POS, nm->localPlayerId, nm->buf + sizeof(MessageHeader), header.size);
        return true;
    } else {
        // Client sends to host
        bytesSent = SDLNet_TCP_Send(nm->client, nm->buf, offset);
        return bytesSent == offset;
    }
}

// Send player shoot event to the network
bool sendPlayerShoot(NetMgr *nm, float x, float y, float angle) {
    MessageHeader header = {
        .type = MSG_SHOOT,
        .playerId = nm->localPlayerId,
        .size = sizeof(float) * 3
    };
    
    // Pack header and data
    int offset = 0;
    memcpy(nm->buf + offset, &header, sizeof(MessageHeader));
    offset += sizeof(MessageHeader);
    
    // Pack shoot data
    memcpy(nm->buf + offset, &x, sizeof(float));
    offset += sizeof(float);
    memcpy(nm->buf + offset, &y, sizeof(float));
    offset += sizeof(float);
    memcpy(nm->buf + offset, &angle, sizeof(float));
    offset += sizeof(float);
    
    // Send the data
    int bytesSent = 0;
    
    if (nm->isHost) {
        // Host sends to all clients
        for (int i = 0; i < nm->peerCount; i++) {
            bytesSent = SDLNet_TCP_Send(nm->peers[i], nm->buf, offset);
            if (bytesSent < offset) {
                printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
                return false;
            }
        }
        // Also process locally
        handleNetworkData(MSG_SHOOT, nm->localPlayerId, nm->buf + sizeof(MessageHeader), header.size);
        return true;
    } else {
        // Client sends to host
        bytesSent = SDLNet_TCP_Send(nm->client, nm->buf, offset);
        return bytesSent == offset;
    }
} 