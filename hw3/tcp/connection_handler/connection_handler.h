#ifndef TCP_CONNECTION_HANDLER_H
#define TCP_CONNECTION_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "tcp_socket.h"
#include "CbNB.h"
#include "connection_handler.h"
#include "http_request.h"
#include "http_parser.h"


/**
 * @brief Call by server to handle a connection. The handler proxy the traffic to packet destination.
 * The supported requests includes http and https requests.
 *
 * @param server_add    struct containing port and address of server listening socket
 *
 * @return -1 if error otherwise 0
 */
int handle_connection(int passive_sfd);

#endif //TCP_CONNECTION_HANDLER_H
