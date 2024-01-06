#include "connection_handler.h"

#define BUFF_SIZE  1024

// ============================ HELPER FUNCTIONS ========================================
/**
 * @brief  Get next connection from passive socket and init active socket
 *
 * @param passive_sfd         socket listening for connections
 * @param client_addr         client address which is filled by accept
 * @param len_client_addr     client address length which is filled by accept
 * @param active_cli_srv_sfd  active socket to client
 * @return
 */
static int get_next_connection(Fd passive_sfd, struct sockaddr_in *client_addr, socklen_t *len_client_addr,
                               Fd *active_cli_srv_sfd);


/**
 * @brief Init bridge between client and destination
 *
 * @param active_cli_srv_sfd   active socket to client
 * @param client_buffer        buffer which contains client request
 * @param httpRequest          http request struct filled by http_parse_request
 * @param active_srv_dest_sfd  active socket to destination
 * @param dest_addr            destination address which is filled by init_tcp_client_socket
 * @return
 */
static int init_bridge(Fd active_cli_srv_sfd, char *client_buffer, http_request_t *httpRequest, Fd *active_srv_dest_sfd,
                       struct sockaddr_in *dest_addr);


/**
 * @brief transfer data from src_sfd to dst_sfd
 *
 * @param src_sfd  source socket which is read from
 * @param dst_sfd  destination socket which is written to
 * @param buff     buffer used for read and send data
 */
static void execute_bridge(Fd src_sfd, Fd dst_sfd, char *buff);
// ======================================================================================



int handle_connection(int passive_sfd)
{
#pragma omp critical
    CBNB_LOG(INFO, "thread %d waiting for connection", omp_get_thread_num());

    struct sockaddr_in client_addr;
    socklen_t len_client_addr = sizeof(client_addr);
    Fd active_cli_srv_sfd;
    http_request_t httpRequest;
    http_request_init(&httpRequest);
    struct sockaddr_in dest_addr;
    Fd active_srv_dest_sfd;

    char *client_buffer = (char *) calloc(BUFF_SIZE, sizeof(char));
    char *server_buffer = (char *) calloc(BUFF_SIZE, sizeof(char));

    if (get_next_connection(passive_sfd, &client_addr, &len_client_addr, &active_cli_srv_sfd)) return -1;
    if (init_bridge(active_cli_srv_sfd, client_buffer, &httpRequest, &active_srv_dest_sfd, &dest_addr)) return -1;

    // print source and dest sfc
    CBNB_LOG(INFO, "thread %d handling connection %d -> %d", omp_get_thread_num(), active_cli_srv_sfd,
             active_srv_dest_sfd);


    // Task handling dest->server server->client traffic
    #pragma omp task default(none) firstprivate(active_srv_dest_sfd, active_cli_srv_sfd, server_buffer)
    execute_bridge(active_srv_dest_sfd, active_cli_srv_sfd, server_buffer);


    bool isConnect = httpRequest.request_line.method == CONNECT;
    if (isConnect) {
        // Task handling client->server server->client traffic
        #pragma omp task default(none) firstprivate(active_srv_dest_sfd, active_cli_srv_sfd, client_buffer)
        execute_bridge(active_cli_srv_sfd, active_srv_dest_sfd, client_buffer);
    }

    #pragma omp taskwait
    return 0;
}

static int init_bridge(Fd active_cli_srv_sfd, char *client_buffer, http_request_t *httpRequest, Fd *active_srv_dest_sfd,
                       struct sockaddr_in *dest_addr)
{
    ssize_t byte_count = recv(active_cli_srv_sfd, client_buffer, BUFF_SIZE, 0);
    if (byte_count < 0) {
        CBNB_LOG(ERROR, "error reading from socket");
        return -1;
    }

    // parse http request
    http_parse_request(httpRequest, client_buffer, BUFF_SIZE);

    // init socket from server to destination
    ConstString host = httpRequest->request_line.path.host;
    int port = httpRequest->request_line.path.port;
    int status = init_tcp_client_socket(active_srv_dest_sfd, dest_addr, host, port);
    if (status != 0) return -1;

    // send first message to destination
    if (httpRequest->request_line.method != CONNECT) {
        byte_count = send((*active_srv_dest_sfd), client_buffer, byte_count, 0);
        if (byte_count < 0) {
            CBNB_LOG(ERROR, "error sending to destination");
            return -1;
        }
    } else {
        // send 200 OK to client
        ConstString ok = "HTTP/1.1 200 Connection established\r\n\r\n";
        byte_count = send(active_cli_srv_sfd, ok, strlen(ok), 0);
        if (byte_count < 0) {
            CBNB_LOG(ERROR, "error sending to client");
            return -1;
        }
    }

    return 0;
}

static int
get_next_connection(Fd passive_sfd, struct sockaddr_in *client_addr, socklen_t *len_client_addr, Fd *active_cli_srv_sfd)
{
    *active_cli_srv_sfd = accept(passive_sfd, (struct sockaddr *) client_addr, len_client_addr);

    if (*active_cli_srv_sfd < 0) {
        CBNB_LOG(ERROR, "error accepting connection");
        return -1;
    }
    CBNB_LOG(INFO, "connection accepted from %s:%d", inet_ntoa((*client_addr).sin_addr),
             ntohs((*client_addr).sin_port));
    return 0;
}


static void execute_bridge(Fd src_sfd, Fd dst_sfd, char *buff)
{
    ssize_t stat;

    CBNB_LOG(INFO, "thread %d handling ", omp_get_thread_num());

    while (CBNB_SOCK_AVAILABLE(src_sfd) && CBNB_SOCK_AVAILABLE(dst_sfd)) {
        bzero(buff, BUFF_SIZE);
        stat = recv(src_sfd, buff, BUFF_SIZE, 0);
        if (stat < 0) break;
        stat = send(dst_sfd, buff, stat, MSG_NOSIGNAL);
        if (stat < 0) break;
    }

    CBNB_Fd_CLOSE(dst_sfd);
    CBNB_Fd_CLOSE(src_sfd);
    CBNB_LOG(INFO, "connection %d closed", src_sfd);
}

