#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <SDL_net.h>

#define MAX_PACKET_SIZE 512

typedef struct{
    UDPsocket socket;
    UDPpacket *packet;
} UDP_Connection;

int init_udp(UDP_Connection *conn, Uint16 port);
int send_udp(UDP_Connection *conn, void *data, int data_size, IPaddress *dest);
int receive_udp(UDP_Connection *conn, void *buffer, int buffer_size, IPaddress *sender);
void close_udp(UDP_Connection *conn);

#endif