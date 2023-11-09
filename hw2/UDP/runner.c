#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <malloc.h>
#include <omp.h>

#include "dns_loader.h"
#include "udp_socket_binder.h"
#include "data-types/dns_message_t.h"
#include "dns_query_resolver.h"


#ifndef PARALLEL_MODE
#define PARALLEL_MODE 0
#endif


void thread_serve(dns_resource_record_t *dns_resource_records, size_t dns_resource_records_size, int socket_desc,
                  struct sockaddr_in *client_addr);

int main() {
    // Load DNS server data:
    dns_resource_record_t *dns_resource_records = malloc(sizeof(dns_resource_record_t) * 10);
    size_t dns_resource_records_size = 10;
    if (load_dns_server_data("/etc/my_hosts", &dns_resource_records, &dns_resource_records_size) != 0) {
        printf("\033[31m[_ERROR_]\033[0m can not load DNS server data.\n");
        return -1;
    }



    // Create UDP socket:
    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    init_udp_socket(&socket_desc, &server_addr, 4444);


    // Receive client's message:
#pragma omp parallel if(PARALLEL_MODE) default(none)  shared(dns_resource_records, dns_resource_records_size, socket_desc, client_addr) num_threads(omp_get_num_procs())
    {
#pragma omp task default(none) shared(dns_resource_records, dns_resource_records_size, socket_desc, client_addr)
        {
            printf("\033[34m[_INFO_]\033[0m thread %d started\n", omp_get_thread_num());
            thread_serve(dns_resource_records, dns_resource_records_size, socket_desc, &client_addr);
        }
    }


    // Close the socket:
    close(socket_desc);

    return 0;
}

void thread_serve(dns_resource_record_t *dns_resource_records, size_t dns_resource_records_size, int socket_desc,
                  struct sockaddr_in *client_addr) {
    unsigned char client_message[512];
    unsigned char server_message[512];
    while (1) {
        socklen_t client_struct_length = sizeof(*client_addr);
        if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
                     (struct sockaddr *) client_addr, &client_struct_length) < 0) {
            printf("\033[31m[_ERROR_]\033[0m can not receive message.\n");
            continue;
        }

        printf("\033[34m[_INFO_]\033[0m message received by thread : %d\n", omp_get_thread_num());

        printf("\033[34m[_INFO_]\033[0m message received from IP : %s and port : %d\n",
               inet_ntoa((*client_addr).sin_addr), ntohs((*client_addr).sin_port));

        dns_message_t dns_message;
        if (parse_dns_message(&dns_message, client_message, sizeof(client_message))) {
            printf("Can't parse DNS message\n");
            continue;
        }

        dns_message_t response;
        if (resolve_query(&dns_message, &response, dns_resource_records, dns_resource_records_size)) {
            // print warning on error code
            printf("\033[33m[_ERROR_]\033[0m QUERY %hu: encounter", dns_message.header.identifier);
            if (response.header.flags.rcode == 1) printf(" format error\n");
            else if (response.header.flags.rcode == 2) printf(" server failure\n");
            else if (response.header.flags.rcode == 3) printf(" name error\n");
            else if (response.header.flags.rcode == 4) printf(" not implemented\n");
            else if (response.header.flags.rcode == 5) printf(" refused\n");
            else printf(" unknown error\n");
        }
        size_t offset;
        stringify_dns_message(&response, server_message, &offset);


        if (sendto(socket_desc, server_message, offset, 0,
                   (struct sockaddr *) client_addr, client_struct_length) < 0) {
            printf("\033[31m[_ERROR_]\033[0m QUERY %hu: can not send message.\n", response.header.identifier);
            continue;
        }

        printf("\033[32m[_SUCCESS_]\033[0m QUERY %hu: respond sent to query with IP: %s, PORT: %d\n",
               response.header.identifier, inet_ntoa((*client_addr).sin_addr), ntohs((*client_addr).sin_port));
    }
}
