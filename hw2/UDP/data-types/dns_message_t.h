#ifndef UDP_DNS_MESSAGE_T_H
#define UDP_DNS_MESSAGE_T_H

#include "resource_record.h"

typedef struct {
    uint8_t qr: 1;     // 0 - query, 1 - response
    uint8_t opcode: 4; // 0 - standard query, 1 - inverse query, 2 - server status request
    uint8_t aa: 1;     // authoritative answer
    uint8_t tc: 1;     // truncated
    uint8_t rd: 1;     // recursion desired
    uint8_t ra: 1;     // recursion available
    uint8_t z: 3;      // reserved
    uint8_t rcode: 4;  // 0 - no error, 1 - format error, 2 - server failure, 3 - name error, 4 - not implemented, 5 - refused
} dns_message_flags_t;


typedef struct {
    uint16_t identifier;       // identification number
    dns_message_flags_t flags; // flags
    uint16_t qdcount;          // number of question entries
    uint16_t ancount;          // number of answer entries
    uint16_t nscount;          // number of authority entries
    uint16_t arcount;          // number of resource entries
} dns_message_header_t;


typedef struct {
    char *name;
    uint8_t size;
} dns_message_label_t;

typedef struct {
    dns_message_label_t *labels;    // domain name
    uint8_t labels_size;            // number of labels
    dns_rr_type_t type;             // type of query
    uint16_t class;                 // class of query
} dns_message_question_t;

typedef struct {
    unsigned char *name;             // domain name
    dns_rr_type_t type;     // type of query
    uint16_t class;         // class of query
    uint32_t ttl;           // time to live
    uint16_t rdlength;      // length of rdata
    char *rdata;            // rdata
} dns_message_resource_t;


typedef struct {
    dns_message_question_t *questions;
    dns_message_resource_t *answers;
    dns_message_resource_t *authorities;
    dns_message_resource_t *additional;
} dns_message_section_t;

typedef struct {
    dns_message_header_t header;
    dns_message_section_t section;
} dns_message_t;


int parse_dns_message(dns_message_t *dns_message, unsigned char *buffer, size_t buffer_size);

#endif //UDP_DNS_MESSAGE_T_H
