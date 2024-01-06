#include <netinet/in.h>
#include <stdlib.h>

#include "CbNB.h"
#include "tcp_socket.h"
#include "connection_handler.h"


#define HOST "127.0.0.1"
#define PORT 4322
#define THREADS_NUM 12


int main()
{
    int err;
    // create TCP socket (user -> proxy server)
    Fd passive_tcp_sfd;
    struct sockaddr_in server_addr;
    err = init_tcp_server_socket(&passive_tcp_sfd, &server_addr, HOST, PORT, 12);
    if (err) exit(1);

#pragma omp parallel default(none) firstprivate(passive_tcp_sfd) num_threads(THREADS_NUM)
    {
        // use THREAD/2 threads to handle connections
        int tid = omp_get_thread_num();
        if (tid < THREADS_NUM / 2) {
            while (1) {
                handle_connection(passive_tcp_sfd);
            }
        }
    }
    return 0;
}



