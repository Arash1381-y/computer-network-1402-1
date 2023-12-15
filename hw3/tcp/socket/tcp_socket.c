#include "tcp_socket.h"


#define SND_TIMEOUT 10
#define RCV_TIMEOUT 10

// ============================ HELPER FUNCTIONS ========================================
/**
 * set timout for a socket to recv or send a data
 *
 * @param socket_desc        socket file descriptor which is modified
 * @param rcv_timeout_sec    time which cause timeout for a socket that is blocked on read/recv/recvfrom calls
 * @param send_timeout_sec   time which cause timout for a socket  that is blocked on write/send calls
 *
 * @return 0 if success. -1 if error
 */
static int set_timeout(Fd socket_desc, Sec rcv_timeout_sec, Sec send_timeout_sec);
// ======================================================================================


int init_tcp_server_socket(Fd *socket_desc, struct sockaddr_in *server_addr, ConstString host, Port port, int backlog)
{
    *socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (*socket_desc < 0) {
        CBNB_LOG(INFO, "can not create TCP socket.");
        return 1;
    }

    CBNB_LOG(INFO, "TCP server socket created...");

    // Set port and IP:
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);
    server_addr->sin_addr.s_addr = inet_addr(host);

    // Bind to the set port and IP:

    while (bind(*socket_desc, (struct sockaddr *) server_addr, sizeof(*server_addr)) < 0) {
        CBNB_LOG(INFO, "binding failed.");
        server_addr->sin_port = htons(++port);
    }


    CBNB_LOG(INFO, "binding done...");


    // Listen for connection on socket
    if (listen(*socket_desc, backlog) < 0) {
        CBNB_LOG(INFO, "listening failed.");
        return 1;
    }

    set_timeout(*socket_desc, 0, SND_TIMEOUT);

    CBNB_LOG(INFO, "listening on address : %s, port : %d", inet_ntoa(server_addr->sin_addr),
             ntohs(server_addr->sin_port));

    return 0;
}


int init_tcp_client_socket(Fd *socket_desc, struct sockaddr_in *dest_addr, ConstString host, Port port)
{
    struct addrinfo *result, *rp;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    // get string port
    char str_port[6];
    sprintf(str_port, "%d", port);


    int status = getaddrinfo(host, str_port, &hints, &result);
    if (status != 0) {
        CBNB_LOG(INFO, "error getting address info");
        return 1;
    }


    status = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        *socket_desc = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (*socket_desc == -1)
            continue;

        // print IP address of rp
        char ipstr[INET_ADDRSTRLEN];
        struct sockaddr_in *addr = (struct sockaddr_in *) rp->ai_addr;
        inet_ntop(AF_INET, &addr->sin_addr, ipstr, sizeof(ipstr));
        CBNB_LOG(INFO, "trying to connect to %s:%d", ipstr, ntohs(addr->sin_port));

        dest_addr->sin_family = AF_INET;
        dest_addr->sin_port = addr->sin_port;
        dest_addr->sin_addr.s_addr = addr->sin_addr.s_addr;

        status = connect(*socket_desc, (const struct sockaddr *) dest_addr, sizeof(*dest_addr));
        if (status < 0) {
            continue;
        }
        CBNB_LOG(INFO, "connected to %s:%d", ipstr, ntohs(addr->sin_port));
        status = 0;
        break;
    }

    if (status < 0) {
        CBNB_LOG(INFO, "error connecting to destination");
        return 1;
    }

    if (set_timeout(*socket_desc, RCV_TIMEOUT, 0)) {
        return 1;
    }

    freeaddrinfo(result);
    return 0;

}


static int set_timeout(Fd socket_desc, Sec rcv_timeout_sec, Sec send_timeout_sec)
{
    struct timeval tv;
    tv.tv_sec = rcv_timeout_sec;
    tv.tv_usec = 0;
    if (rcv_timeout_sec != 0)
        if (setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof tv) < 0) {
            CBNB_LOG(INFO, "error setting timeout");
            return 1;
        }


    tv.tv_sec = send_timeout_sec;
    if (send_timeout_sec != 0)
        if (setsockopt(socket_desc, SOL_SOCKET, SO_SNDTIMEO, (const char *) &tv, sizeof tv) < 0) {
            CBNB_LOG(INFO, "error setting timeout");
            return 1;
        }
    return 0;
}