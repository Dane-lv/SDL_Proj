#include "../include/network.h"
#include "../include/game_core.h"
#include <string.h>
#include <stdio.h>

// Vidarebefordrar nätverksmeddelanden till spelmotorn
static void dispatchMessage(NetMgr *nm, Uint8 type, Uint8 playerId, const void *data, int size) {
    if (nm->userData) {
        GameContext *ctx = (GameContext*)nm->userData;
        gameOnNetworkMessage(ctx, type, playerId, data, size);
    }
}

// Initierar SDL_net
bool netInit(void) {
    return SDLNet_Init() == 0;
}

// Stänger ner SDL_net
void netShutdown(void) {
    SDLNet_Quit();
}

// Startar en värddator som lyssnar efter anslutningar
bool hostStart(NetMgr *nm, int port) {
    IPaddress ip;
    
    // Konfigurera IP-adress för att lyssna
    if (SDLNet_ResolveHost(&ip, NULL, port) < 0) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return false;
    }
    
    // Skapa socket-uppsättning för att hantera anslutningar
    nm->set = SDLNet_AllocSocketSet(MAX_PLAYERS + 1);
    if (!nm->set) {
        printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        return false;
    }
    
    // Skapa server-socket
    nm->server = SDLNet_TCP_Open(&ip);
    if (!nm->server) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        SDLNet_FreeSocketSet(nm->set);
        return false;
    }
    
    // Lägg till server-socket i uppsättningen
    SDLNet_TCP_AddSocket(nm->set, nm->server);
    
    // Initiera anslutningsräknare och spelar-ID
    nm->peerCount = 0;
    nm->isHost = true;
    nm->localPlayerId = 0;  // Värden är alltid spelare 0
    
    printf("Värddator startad på port %d\n", port);
    return true;
}

// Uppdaterar värddatorn (anropas varje bildruta)
void hostTick(NetMgr *nm, void *ctx) {
    nm->userData = ctx;
    
    // Kontrollera om någon socket har data att läsa
    int ready = SDLNet_CheckSockets(nm->set, 0);
    if (ready <= 0) return;

    // Acceptera nya anslutningar
    if (SDLNet_SocketReady(nm->server) && nm->peerCount < MAX_PLAYERS-1) {
        TCPsocket newClient = SDLNet_TCP_Accept(nm->server);
        if (newClient) {
            // Tilldela ett spelar-ID till den nya klienten
            Uint8 newPlayerId = nm->peerCount + 1;
            
            // Lagra klientens socket
            nm->peers[nm->peerCount++] = newClient;
            SDLNet_TCP_AddSocket(nm->set, newClient);
            
            // Skicka bekräftelse med tilldelat spelar-ID
            MessageHeader header = {
                .type = MSG_JOIN,
                .playerId = newPlayerId,
                .size = 0
            };
            
            memcpy(nm->buf, &header, sizeof(MessageHeader));
            SDLNet_TCP_Send(newClient, nm->buf, sizeof(MessageHeader));
            
            printf("Ny klient ansluten, ID: %d\n", newPlayerId);
        }
        ready--;
    }

    // Ta emot data från klienter och vidarebefordra
    for (int i = 0; i < nm->peerCount && ready > 0; ++i) {
        TCPsocket s = nm->peers[i];
        if (SDLNet_SocketReady(s)) {
            int len = SDLNet_TCP_Recv(s, nm->buf, BUF_SIZE);
            
            if (len <= 0) {
                // Klienten har kopplat från
                printf("Klient %d kopplade från\n", i + 1);
                SDLNet_TCP_DelSocket(nm->set, s);
                SDLNet_TCP_Close(s);
                
                // Flytta resterande klienter för att fylla luckan
                if (i < nm->peerCount - 1) {
                    nm->peers[i] = nm->peers[nm->peerCount - 1];
                }
                nm->peerCount--;
                i--;
            } 
            else if ((unsigned int)len >= sizeof(MessageHeader)) {
                // Bearbeta mottagen data
                MessageHeader *header = (MessageHeader*)nm->buf;
                
                // Bearbeta meddelandet lokalt först
                dispatchMessage(nm, header->type, header->playerId, 
                               nm->buf + sizeof(MessageHeader), 
                               header->size);
                
                // Vidarebefordra till alla andra klienter
                for (int j = 0; j < nm->peerCount; ++j) {
                    if (j != i) {  // Skicka inte tillbaka till avsändaren
                        SDLNet_TCP_Send(nm->peers[j], nm->buf, len);
                    }
                }
            }
            ready--;
        }
    }
}

// Ansluter en klient till en värddator
bool clientConnect(NetMgr *nm, const char *ip, int port) {
    IPaddress serverIP;
    
    // Hämta serverns IP-adress
    if (SDLNet_ResolveHost(&serverIP, ip, port) < 0) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return false;
    }
    
    // Skapa klientsocket och anslut till servern
    nm->client = SDLNet_TCP_Open(&serverIP);
    if (!nm->client) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        return false;
    }
    
    // Skapa socket-uppsättning för att övervaka anslutningen
    nm->set = SDLNet_AllocSocketSet(1);
    if (!nm->set) {
        printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        SDLNet_TCP_Close(nm->client);
        return false;
    }
    
    // Lägg till klientsocketen i uppsättningen
    SDLNet_TCP_AddSocket(nm->set, nm->client);
    
    // Initiera klientstatus
    nm->isHost = false;
    nm->localPlayerId = 0xFF;  // Ogiltigt ID tills det tilldelas av värden
    
    printf("Ansluten till server på %s:%d\n", ip, port);
    return true;
}

// Uppdaterar klienten (anropas varje bildruta)
void clientTick(NetMgr *nm, void *ctx) {
    nm->userData = ctx;
    
    // Kontrollera om socketen har data att läsa
    int ready = SDLNet_CheckSockets(nm->set, 0);
    if (ready <= 0) return;
    
    if (SDLNet_SocketReady(nm->client)) {
        // Ta emot data från servern
        int len = SDLNet_TCP_Recv(nm->client, nm->buf, BUF_SIZE);
        
        if (len <= 0) {
            // Frånkopplad från servern
            printf("Frånkopplad från servern\n");
            return;
        } 
        else if ((unsigned int)len >= sizeof(MessageHeader)) {
            MessageHeader *header = (MessageHeader*)nm->buf;
            
            // Kontrollera om detta är en JOIN-bekräftelse
            if (header->type == MSG_JOIN && nm->localPlayerId == 0xFF) {
                nm->localPlayerId = header->playerId;
                printf("Tilldelat spelar-ID: %d\n", nm->localPlayerId);
            }
            
            // Bearbeta mottagen data
            dispatchMessage(nm, header->type, header->playerId,
                          nm->buf + sizeof(MessageHeader),
                          header->size);
        }
    }
}

// Skickar spelarens position till nätverket
bool sendPlayerPosition(NetMgr *nm, float x, float y, float angle) {
    // Skapa meddelandehuvud
    MessageHeader header = {
        .type = MSG_POS,
        .playerId = nm->localPlayerId,
        .size = sizeof(float) * 3
    };
    
    // Kopiera huvud och positionsdata till bufferten
    memcpy(nm->buf, &header, sizeof(MessageHeader));
    
    float* posData = (float*)(nm->buf + sizeof(MessageHeader));
    posData[0] = x;
    posData[1] = y;
    posData[2] = angle;
    
    int totalSize = sizeof(MessageHeader) + sizeof(float) * 3;
    
    // Skicka data baserat på om det är värd eller klient
    if (nm->isHost) {
        // Värden skickar till alla klienter
        for (int i = 0; i < nm->peerCount; i++) {
            if (SDLNet_TCP_Send(nm->peers[i], nm->buf, totalSize) < totalSize) {
                return false;
            }
        }
        // Bearbeta också lokalt
        dispatchMessage(nm, MSG_POS, nm->localPlayerId, posData, header.size);
        return true;
    } else {
        // Klienten skickar bara till värden
        return SDLNet_TCP_Send(nm->client, nm->buf, totalSize) == totalSize;
    }
}

// Skickar spelarens skotthändelse till nätverket
bool sendPlayerShoot(NetMgr *nm, float x, float y, float angle) {
    // Skapa meddelandehuvud
    MessageHeader header = {
        .type = MSG_SHOOT,
        .playerId = nm->localPlayerId,
        .size = sizeof(float) * 3
    };
    
    // Kopiera huvud och skottdata till bufferten
    memcpy(nm->buf, &header, sizeof(MessageHeader));
    
    float* shootData = (float*)(nm->buf + sizeof(MessageHeader));
    shootData[0] = x;
    shootData[1] = y;
    shootData[2] = angle;
    
    int totalSize = sizeof(MessageHeader) + sizeof(float) * 3;
    
    // Skicka data baserat på om det är värd eller klient
    if (nm->isHost) {
        // Värden skickar till alla klienter
        for (int i = 0; i < nm->peerCount; i++) {
            if (SDLNet_TCP_Send(nm->peers[i], nm->buf, totalSize) < totalSize) {
                return false;
            }
        }
        // Bearbeta också lokalt
        dispatchMessage(nm, MSG_SHOOT, nm->localPlayerId, shootData, header.size);
        return true;
    } else {
        // Klienten skickar bara till värden
        return SDLNet_TCP_Send(nm->client, nm->buf, totalSize) == totalSize;
    }
}