#include <netdb.h>
#include <string.h>
#include "tcp_socket.h"


int init_tcp_server_socket(int *socket_desc, struct sockaddr_in *server_addr, char *host, int port, logger_t *l,
                           int backlog) {
    *socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (*socket_desc < 0) {
        error(l, "can not create TCP socket.");
        return 1;
    }

    info(l, "TCP server socket created...");

    // Set port and IP:
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);
    server_addr->sin_addr.s_addr = inet_addr(host);

    // Bind to the set port and IP:

    while (bind(*socket_desc, (struct sockaddr *) server_addr, sizeof(*server_addr)) < 0) {
        error(l, "binding failed.");
        server_addr->sin_port = htons(++port);
    }


    info(l, "binding done...");


    // Listen for connection on socket
    if (listen(*socket_desc, backlog) < 0) {
        error(l, "listening failed.");
        return 1;
    }

    info(l, "listening on address : %s, port : %d",
         inet_ntoa(server_addr->sin_addr),
         ntohs(server_addr->sin_port));

    return 0;
}


int init_tcp_client_socket(int *socket_desc, struct sockaddr_in *server_addr, char *host, int port, logger_t *l) {
    *socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (*socket_desc < 0) {
        error(l, "can not create TCP socket.");
        return 1;
    }

    info(l, "TCP client socket created...");

    // Set port and IP:
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);

    // get address of host
    struct hostent *host_entry;
    host_entry = gethostbyname(host);
    if (host_entry == NULL) {
        error(l, "can not get host by name.");
        return 1;
    }

    // copy address to server_addr
    memcpy(&server_addr->sin_addr, host_entry->h_addr_list[0], host_entry->h_length);

    info(l, "sending to address : %s, port : %d",
         inet_ntoa(server_addr->sin_addr),
         ntohs(server_addr->sin_port));

    return 0;
}

