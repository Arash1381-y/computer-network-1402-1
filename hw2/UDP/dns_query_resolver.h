//
// Created by halfblood on 11/9/23.
//

#ifndef UDP_DNS_QUERY_RESOLVER_H
#define UDP_DNS_QUERY_RESOLVER_H


#include <malloc.h>
#include <string.h>

#include "data-types/dns_message_t.h"
#include "data-types/resource_record.h"


int resolve_query(dns_message_t *dns_message, dns_message_t *response, dns_resource_record_t *dns_resource_records,
                  size_t dns_resource_records_size);

int stringify_dns_message(dns_message_t *dns_message, unsigned char *buffer, size_t *offset);

#endif //UDP_DNS_QUERY_RESOLVER_H
