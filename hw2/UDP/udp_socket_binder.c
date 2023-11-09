#include "udp_socket_binder.h"

int init_udp_socket(int *socket_desc, struct sockaddr_in *server_addr, int port) {
    *socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (*socket_desc < 0) {
        printf("\033[31m[_ERROR_]\033[0m can not create UDP socket.\n");
        return 1;
    }


    printf("\033[34m[_INFO_]\033[0m UDP socket created...\n");

    // Set port and IP:
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);
    server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind to the set port and IP:
    if (bind(*socket_desc, (struct sockaddr *) server_addr, sizeof(*server_addr)) < 0) {
        printf("\033[31m[_ERROR_]\033[0m binding failed.\n");
        return 1;
    }

    printf("\033[34m[_INFO_]\033[0m binding done...\n");
    printf("\033[34m[_INFO_]\033[0m waiting for messages on port %d...\n", port);
    return 0;
}