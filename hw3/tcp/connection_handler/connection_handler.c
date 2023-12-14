#include <string.h>
#include <unistd.h>
#include "connection_handler.h"
#include "../data_types/http_request.h"
#include "../http_parser/http_parser.h"

#define BUFF_SIZE  1024


// ============================ HELPER FUNCTIONS========================================
static int get_next_connection(int passive_sfd, logger_t *l, struct sockaddr_in *client_addr, socklen_t *lclient_addr,
                               int *active_cli_srv_sfd);

static void execute_bridge(int src_sfd, int dst_sfd, char *buff, logger_t *l);

static int init_bridge(logger_t *l, int active_cli_srv_sfd, char *client_buffer, http_request_t *httpRequest,
                       int *active_srv_dest_sfd, struct sockaddr_in *dest_addr);
// =====================================================================================



int handle_connection(int passive_sfd, logger_t *l) {

    struct sockaddr_in client_addr;
    socklen_t lclient_addr = sizeof(client_addr);
    int active_cli_srv_sfd;


    if (get_next_connection(passive_sfd, l, &client_addr, &lclient_addr, &active_cli_srv_sfd)) return -1;

    http_request_t httpRequest;
    struct sockaddr_in dest_addr;
    int active_srv_dest_sfd;

    char *client_buffer = (char *) malloc(BUFF_SIZE);
    char *server_buffer = (char *) malloc(BUFF_SIZE);
    bzero(client_buffer, BUFF_SIZE);
    bzero(server_buffer, BUFF_SIZE);

    if (init_bridge(l, active_cli_srv_sfd, client_buffer, &httpRequest, &active_srv_dest_sfd, &dest_addr)) return -1;

    // Task handling dest->server server->client traffic
#pragma omp task default(none) firstprivate(active_srv_dest_sfd, active_cli_srv_sfd, server_buffer, l)
    {
        execute_bridge(active_srv_dest_sfd, active_cli_srv_sfd, server_buffer, l);
    }

    // Task handling client->server server->dest traffic
#pragma omp task default(none) firstprivate(active_srv_dest_sfd, active_cli_srv_sfd, client_buffer, l) if(httpRequest.request_line.method == CONNECT)
    {
        execute_bridge(active_cli_srv_sfd, active_srv_dest_sfd, client_buffer, l);
    }

#pragma omp taskwait
    return 0;
}

static int init_bridge(logger_t *l, int active_cli_srv_sfd, char *client_buffer, http_request_t *httpRequest,
                       int *active_srv_dest_sfd, struct sockaddr_in *dest_addr) {
    ssize_t byte_count = recv(active_cli_srv_sfd, client_buffer, BUFF_SIZE, 0);
    if (byte_count < 0) {
        error(l, "error reading from socket");
        return -1;
    }

    // parse http request
    http_parse_request(httpRequest, client_buffer, BUFF_SIZE);

    // init socket from server to destination
    init_tcp_client_socket(active_srv_dest_sfd, dest_addr,
                           httpRequest->request_line.path.host,
                           httpRequest->request_line.path.port,
                           l);
    int status = connect((*active_srv_dest_sfd), (const struct sockaddr *) dest_addr, sizeof(*dest_addr));
    if (status < 0) {
        error(l, "error connecting to destination");
        return -1;
    }

    // send first message to destination
    if (httpRequest->request_line.method != CONNECT) {
        byte_count = send((*active_srv_dest_sfd), client_buffer, byte_count, 0);
        if (byte_count < 0) {
            error(l, "error sending to destination");
            return -1;
        }
    } else {
        // send 200 OK to client
        char *ok = "HTTP/1.1 200 Connection established\r\n\r\n";
        byte_count = send(active_cli_srv_sfd, ok, strlen(ok), 0);
        if (byte_count < 0) {
            error(l, "error sending to client");
            return -1;
        }
    }

    return 0;
}

static int get_next_connection(int passive_sfd, logger_t *l, struct sockaddr_in *client_addr, socklen_t *lclient_addr,
                               int *active_cli_srv_sfd) {
    *active_cli_srv_sfd = accept(passive_sfd, (struct sockaddr *) client_addr, lclient_addr);
    if (active_cli_srv_sfd < 0) {
        error(l, "error accepting connection");
        return -1;
    }
    info(l, "connection accepted from %s:%d",
         inet_ntoa((*client_addr).sin_addr),
         ntohs((*client_addr).sin_port));
    return 0;
}


static void execute_bridge(int src_sfd, int dst_sfd, char *buff, logger_t *l) {
    info(l, "thread %d handling ", omp_get_thread_num());
    ssize_t bytes_read;
    while (true) {
        bzero(buff, BUFF_SIZE);
        bytes_read = recv(src_sfd, buff, BUFF_SIZE, 0);
        if (bytes_read <= 0) {
            info(l, "closing connection %d", src_sfd);
            break;
        }
        send(dst_sfd, buff, bytes_read, 0);
    }
    close(src_sfd);
}

