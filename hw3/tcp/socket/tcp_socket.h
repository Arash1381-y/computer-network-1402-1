#ifndef TCP_TCP_SOCKET_H
#define TCP_TCP_SOCKET_H


#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <omp.h>
#include "../logger/logger.h"


int init_tcp_server_socket(int *socket_desc, struct sockaddr_in *server_addr, char *host, int port, logger_t *l,
                           int backlog);

int init_tcp_client_socket(int *socket_desc, struct sockaddr_in *server_addr, char *host, int port, logger_t *l);


#endif //TCP_TCP_SOCKET_H