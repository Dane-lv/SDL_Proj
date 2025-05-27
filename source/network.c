#include "../include/network.h"
#include "../include/game_core.h"
#include <string.h>
#include <stdio.h>

static void dispatchMessage(NetMgr *nm, Uint8 type, Uint8 playerId,
                            const void *data, int size)
{
    if (nm->userData)
    {
        GameContext *g = (GameContext *)nm->userData;
        gameOnNetworkMessage(g, type, playerId, data, size);
    }
}

static void processBuffer(NetMgr *nm, const char *buf, int len)
{
    int off = 0;
    while (off + (int)sizeof(MessageHeader) <= len)
    {
        const MessageHeader *h = (const MessageHeader *)(buf + off);
        int full = sizeof(MessageHeader) + h->size;
        if (off + full > len)
            break;
        if (!nm->isHost && h->type == MSG_JOIN)
        {
            if (nm->localPlayerId == 0xFF)
            {
                nm->localPlayerId = h->playerId;
                nm->peerCount = 1;
            }
            else if (h->playerId != 0)
            {
                ++nm->peerCount;
            }
        }

        dispatchMessage(nm, h->type, h->playerId,
                        buf + off + sizeof(MessageHeader), h->size);

        off += full;
    }
}
bool netInit(void) { return SDLNet_Init() == 0; }
void netShutdown(void) { SDLNet_Quit(); }

bool hostStart(NetMgr *nm, int port)
{
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, NULL, port) < 0)
        return false;

    nm->set = SDLNet_AllocSocketSet(MAX_PLAYERS + 1);
    if (!nm->set)
        return false;

    nm->server = SDLNet_TCP_Open(&ip);
    if (!nm->server)
    {
        SDLNet_FreeSocketSet(nm->set);
        return false;
    }

    SDLNet_TCP_AddSocket(nm->set, nm->server);
    nm->peerCount = 0;
    nm->isHost = true;
    nm->localPlayerId = 0;
    return true;
}

void hostTick(NetMgr *nm, void *game)
{
    nm->userData = game;
    int ready = SDLNet_CheckSockets(nm->set, 0);
    if (ready <= 0)
        return;

    if (SDLNet_SocketReady(nm->server) && nm->peerCount < MAX_PLAYERS - 1)
    {
        TCPsocket c = SDLNet_TCP_Accept(nm->server);
        if (c)
        {
            Uint8 newId = nm->peerCount + 1;
            nm->peers[nm->peerCount++] = c;
            SDLNet_TCP_AddSocket(nm->set, c);

            MessageHeader h = {MSG_JOIN, newId, 0};
            memcpy(nm->buf, &h, sizeof h);

            SDLNet_TCP_Send(c, nm->buf, sizeof h);
            for (int i = 0; i < nm->peerCount - 1; ++i)
                SDLNet_TCP_Send(nm->peers[i], nm->buf, sizeof h);
            dispatchMessage(nm, MSG_JOIN, newId, NULL, 0);
            for (Uint8 id = 0; id < newId; ++id)
            {
                MessageHeader j = {MSG_JOIN, id, 0};
                memcpy(nm->buf, &j, sizeof j);
                SDLNet_TCP_Send(c, nm->buf, sizeof j);
            }
        }
        --ready;
    }

    for (int i = 0; i < nm->peerCount && ready > 0; ++i)
    {
        TCPsocket s = nm->peers[i];
        if (SDLNet_SocketReady(s))
        {
            int len = SDLNet_TCP_Recv(s, nm->buf, BUF_SIZE);

            if (len <= 0)
            {
                SDLNet_TCP_DelSocket(nm->set, s);
                SDLNet_TCP_Close(s);
                nm->peers[i] = nm->peers[--nm->peerCount];
                --i;
            }
            else
            {
                processBuffer(nm, nm->buf, len);

                for (int j = 0; j < nm->peerCount; ++j)
                    if (j != i)
                        SDLNet_TCP_Send(nm->peers[j], nm->buf, len);
            }
            --ready;
        }
    }
}

bool clientConnect(NetMgr *nm, const char *ip, int port)
{
    IPaddress srv;
    if (SDLNet_ResolveHost(&srv, ip, port) < 0)
        return false;

    nm->client = SDLNet_TCP_Open(&srv);
    if (!nm->client)
        return false;

    nm->set = SDLNet_AllocSocketSet(1);
    if (!nm->set)
    {
        SDLNet_TCP_Close(nm->client);
        return false;
    }

    SDLNet_TCP_AddSocket(nm->set, nm->client);
    nm->isHost = false;
    nm->localPlayerId = 0xFF;
    nm->peerCount = 0;
    return true;
}

void clientTick(NetMgr *nm, void *game)
{
    nm->userData = game;
    if (SDLNet_CheckSockets(nm->set, 0) <= 0)
        return;

    if (SDLNet_SocketReady(nm->client))
    {
        int len = SDLNet_TCP_Recv(nm->client, nm->buf, BUF_SIZE);
        if (len <= 0)
            return;

        processBuffer(nm, nm->buf, len);
    }
}

static bool sendToAll(NetMgr *nm, int total)
{
    for (int i = 0; i < nm->peerCount; ++i)
        if (SDLNet_TCP_Send(nm->peers[i], nm->buf, total) < total)
            return false;
    return true;
}

bool sendPlayerPosition(NetMgr *nm, float x, float y, float a)
{
    MessageHeader h = {MSG_POS, nm->localPlayerId, sizeof(float) * 3};
    memcpy(nm->buf, &h, sizeof h);
    float *d = (float *)(nm->buf + sizeof h);
    d[0] = x;
    d[1] = y;
    d[2] = a;
    int tot = sizeof h + sizeof(float) * 3;

    if (nm->isHost)
    {
        sendToAll(nm, tot);
        dispatchMessage(nm, MSG_POS, nm->localPlayerId, d, h.size);
        return true;
    }
    return SDLNet_TCP_Send(nm->client, nm->buf, tot) == tot;
}

bool sendPlayerShoot(NetMgr *nm, float x, float y, float a, int pid)
{
    MessageHeader h = {MSG_SHOOT, nm->localPlayerId, sizeof(float) * 3 + sizeof(int)};
    memcpy(nm->buf, &h, sizeof h);
    float *d = (float *)(nm->buf + sizeof h);
    d[0] = x;
    d[1] = y;
    d[2] = a;
    int *id = (int *)(nm->buf + sizeof h + sizeof(float) * 3);
    *id = pid;
    int tot = sizeof h + sizeof(float) * 3 + sizeof(int);

    if (nm->isHost)
    {
        sendToAll(nm, tot);
        dispatchMessage(nm, MSG_SHOOT, nm->localPlayerId, d, h.size);
        return true;
    }
    return SDLNet_TCP_Send(nm->client, nm->buf, tot) == tot;
}

bool sendPlayerDeath(NetMgr *nm, Uint8 killerId)
{
    MessageHeader h = {MSG_DEATH, nm->localPlayerId, sizeof(Uint8)};
    memcpy(nm->buf, &h, sizeof h);
    nm->buf[sizeof h] = killerId;
    int tot = sizeof h + sizeof(Uint8);

    if (nm->isHost)
    {
        sendToAll(nm, tot);
        dispatchMessage(nm, MSG_DEATH, nm->localPlayerId,
                        nm->buf + sizeof h, h.size);
        return true;
    }
    return SDLNet_TCP_Send(nm->client, nm->buf, tot) == tot;
}

bool sendStartGame(NetMgr *nm)
{
    MessageHeader h = {MSG_START, nm->localPlayerId, 0};
    memcpy(nm->buf, &h, sizeof h);

    if (nm->isHost)
    {
        sendToAll(nm, sizeof h);
        dispatchMessage(nm, MSG_START, nm->localPlayerId, NULL, 0);
        return true;
    }
    return false;
}