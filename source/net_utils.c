#include <SDL_net.h>

#include "../include/net_utils.h"
#include "../include/constants.h"

int init_udp(UDP_Connection *conn, Uint16 port){
    conn->socket = SDLNet_UDP_Open(port);
    if(!conn->socket)
        return -1;

    conn->packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if(!conn->packet){
        SDLNet_UDP_Close(conn->socket);
        return -2;
    }

    return 0;
}

int send_udp(UDP_Connection *conn, void *data, int data_size, IPaddress *dest){
    if(data_size > MAX_PACKET_SIZE)
        return -1;

    memcpy((char*) conn->packet->data, data, data_size);
    conn->packet->len = data_size;
    conn->packet->address = *dest;

    return SDLNet_UDP_Send(conn->socket, -1, conn->packet);
}

int receive_udp(UDP_Connection *conn, void *buffer, int buffer_size, IPaddress *sender){
    if(SDLNet_UDP_Recv(conn->socket, conn->packet)){
        if(sender)
            *sender = conn->packet->address;
        memcpy(buffer, conn->packet->data, buffer_size);
        
        return conn->packet->len;
    }

    return 0;
}

void close_udp(UDP_Connection *conn){
    if(conn->packet)
        SDLNet_FreePacket(conn->packet);
    if(conn->socket)
        SDLNet_UDP_Close(conn->socket);
}