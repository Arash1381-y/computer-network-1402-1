#ifndef TCP_TCP_SOCKET_H
#define TCP_TCP_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <omp.h>
#include <netdb.h>
#include <string.h>

#include "CbNB.h"


/**
 * @brief initialize a tcp server socket with given host and port. The maximum number of connections is equal to backlog.
 * The socket is bind and set to passive/listen mode to wait for connections.
 *
 * @param socket_desc  socket to bind
 * @param server_addr  socket address struct to save host and port
 * @param host         host in numerical form (B.B.B.B) which socket try to bind to it
 * @param port         port in numerical from which socket try to bind to it
 * @param backlog      maximum number of connections waiting in queue for connecting to socket
 *
 * @return             0 if success. 1 if failed
 */
int init_tcp_server_socket(Fd *socket_desc, struct sockaddr_in *server_addr, ConstString host, Port port, int backlog);


/**
 * @brief initialize a tcp client socket with given host and port.
 *
 * @param socket_desc  socket to connect with
 * @param dest_addr    the address which is desired to connect to it
 * @param host         host in NON numerical form
 * @param port         Port in numerical form
 *
 * @return             0 if success. 1 if failed
 */
int init_tcp_client_socket(Fd *socket_desc, struct sockaddr_in *dest_addr, ConstString host, Port port);


#endif //TCP_TCP_SOCKET_H