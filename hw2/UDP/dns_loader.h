#ifndef UDP_DNS_LOADER_H
#define UDP_DNS_LOADER_H


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <arpa/inet.h>

#include "data-types/resource_record.h"

static char *DEFAULT_DNS_SERVERS_PATH = "/etc/hosts";

int load_dns_server_data(const char *path, dns_resource_record_t **dns_resource_records,
                         size_t *dns_resource_records_size);

#endif //UDP_DNS_LOADER_H
