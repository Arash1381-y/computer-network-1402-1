#ifndef UDP_UDP_SOCKET_BINDER_H
#define UDP_UDP_SOCKET_BINDER_H


#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>

int init_udp_socket(int *socket_desc, struct sockaddr_in *server_addr, int port);

#endif //UDP_UDP_SOCKET_BINDER_H
