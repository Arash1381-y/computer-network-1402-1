#ifndef TCP_CONNECTION_HANDLER_H
#define TCP_CONNECTION_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <netinet/in.h>

#include "../data_types/buffer.h"
#include "../socket/tcp_socket.h"
#include "../data_types/buffer.h"
#include "../logger/logger.h"


/**
 * Call by server to handle a connection. The handler proxy the traffic to packet destination.
 * The supported requests includes http and https requests.
 *
 * @param server_add struct containing port and address of server listening socket
 * @param l logger
 *
 * @return -1 if error otherwise 0
 */
int handle_connection(int passive_sfd, logger_t *l);

#endif //TCP_CONNECTION_HANDLER_H
