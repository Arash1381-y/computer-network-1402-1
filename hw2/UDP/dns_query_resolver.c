#include <netinet/in.h>
#include <stdbool.h>
#include "dns_query_resolver.h"




// ============================ HELPER FUNCTIONS========================================
static int set_header(dns_message_t *dns_message, dns_message_t *response);

static int set_answer(dns_message_t *dns_message, dns_message_t *response, dns_resource_record_t *dns_resource_records,
                      size_t dns_resource_records_size);
// =====================================================================================

int resolve_query(dns_message_t *dns_message, dns_message_t *response, dns_resource_record_t *dns_resource_records,
                  size_t dns_resource_records_size) {

    bool error = false;
    // set header
    error = set_header(dns_message, response);

    // copy question
    response->section.questions = malloc(sizeof(dns_message_question_t));
    response->section.questions[0].labels_size = dns_message->section.questions[0].labels_size;
    response->section.questions[0].labels = malloc(
            sizeof(dns_message_label_t) * dns_message->section.questions[0].labels_size);
    memcpy(response->section.questions[0].labels, dns_message->section.questions[0].labels,
           sizeof(dns_message_label_t) * dns_message->section.questions[0].labels_size);
    response->section.questions[0].type = dns_message->section.questions[0].type;
    response->section.questions[0].class = dns_message->section.questions[0].class;



    // set counts
    response->header.qdcount = 1;
    response->header.nscount = 0;
    response->header.arcount = 0;
    response->header.ancount = 1;

    if (error) {
        response->header.ancount = 0;
        response->header.arcount = 0;
        response->header.nscount = 0;
        return 1;
    }


    // set answer
    error = set_answer(dns_message, response, dns_resource_records, dns_resource_records_size);
    if (error) {
        response->header.ancount = 0;
        response->header.arcount = 0;
        response->header.nscount = 0;
        return 1;
    }

    response->header.flags.rcode = 0;
    return error;
}


uint16_t concatenateFlags(dns_message_flags_t flags) {
    uint16_t result = 0;

    // Concatenating the fields into a 16-bit number
    result |= (flags.qr & 0x01) << 15;  // Place the qr bit in the 15th position
    result |= (flags.opcode & 0x0F) << 11;  // Place the opcode bits in the 11th to 8th positions
    result |= (flags.aa & 0x01) << 10;  // Place the aa bit in the 10th position
    result |= (flags.tc & 0x01) << 9;  // Place the tc bit in the 9th position
    result |= (flags.rd & 0x01) << 8;  // Place the rd bit in the 8th position
    result |= (flags.ra & 0x01) << 7;  // Place the ra bit in the 7th position
    result |= (flags.z & 0x07) << 4;  // Place the z bits in the 6th to 4th positions
    result |= (flags.rcode & 0x0F);  // Place the rcode bits in the 3rd to 0th positions

    return result;
}


int stringify_dns_message(dns_message_t *dns_message, unsigned char *buffer, size_t *offset) {

    // set header
    uint16_t identifier = htons(dns_message->header.identifier);
    memcpy(buffer, &identifier, sizeof(uint16_t));

    uint16_t flags = htons(concatenateFlags(dns_message->header.flags));
    memcpy(buffer + sizeof(uint16_t), &flags, sizeof(uint16_t));

    uint16_t qdcount = htons(dns_message->header.qdcount);
    memcpy(buffer + sizeof(uint16_t) * 2, &qdcount, sizeof(uint16_t));
    uint16_t ancount = htons(dns_message->header.ancount);
    memcpy(buffer + sizeof(uint16_t) * 3, &ancount, sizeof(uint16_t));
    uint16_t nscount = htons(dns_message->header.nscount);
    memcpy(buffer + sizeof(uint16_t) * 4, &nscount, sizeof(uint16_t));
    uint16_t arcount = htons(dns_message->header.arcount);
    memcpy(buffer + sizeof(uint16_t) * 5, &arcount, sizeof(uint16_t));



    // set question
    int offset_val = sizeof(uint16_t) * 6;
    for (int i = 0; i < dns_message->section.questions[0].labels_size; i += 1) {
        buffer[offset_val] = dns_message->section.questions[0].labels[i].size;
        offset_val += 1;
        memcpy(buffer + offset_val, dns_message->section.questions[0].labels[i].name,
               dns_message->section.questions[0].labels[i].size);
        offset_val += dns_message->section.questions[0].labels[i].size;
    }

    buffer[offset_val] = 0;
    offset_val += 1;


    uint16_t type = htons(dns_message->section.questions[0].type);
    memcpy(buffer + offset_val, &type, sizeof(uint16_t));
    offset_val += sizeof(uint16_t);
    uint16_t class = htons(dns_message->section.questions[0].class);
    memcpy(buffer + offset_val, &class, sizeof(uint16_t));
    offset_val += sizeof(uint16_t);

    if (dns_message->header.flags.rcode != 0) {
        *offset = offset_val;
        return 0;
    }

    // set answer
    buffer[offset_val] = dns_message->section.answers[0].name[0];
    offset_val += 1;
    buffer[offset_val] = dns_message->section.answers[0].name[1];
    offset_val += 1;
    uint16_t type2 = htons(*(uint16_t *) &dns_message->section.answers[0].type);
    memcpy(buffer + offset_val, &type2, sizeof(uint16_t));
    offset_val += sizeof(uint16_t);
    uint16_t class2 = htons(dns_message->section.answers[0].class);
    memcpy(buffer + offset_val, &class2, sizeof(uint16_t));
    offset_val += sizeof(uint16_t);
    uint32_t ttl = htonl(dns_message->section.answers[0].ttl);
    memcpy(buffer + offset_val, &ttl, sizeof(uint32_t));
    offset_val += sizeof(uint32_t);
    uint16_t rdlength = htons(dns_message->section.answers[0].rdlength);
    memcpy(buffer + offset_val, &rdlength, sizeof(uint16_t));
    offset_val += sizeof(uint16_t);
    memcpy(buffer + offset_val, dns_message->section.answers[0].rdata, dns_message->section.answers[0].rdlength);
    offset_val += dns_message->section.answers[0].rdlength;

    *offset = offset_val;

    return 0;
}


static int set_header(dns_message_t *dns_message, dns_message_t *response) {

    // Set identifier
    response->header.identifier = dns_message->header.identifier;

    // Set flags
    response->header.flags.qr = 1;
    response->header.flags.opcode = dns_message->header.flags.opcode;
    response->header.flags.aa = 1;
    response->header.flags.tc = 0;
    response->header.flags.rd = dns_message->header.flags.rd;
    response->header.flags.ra = 0;
    response->header.flags.z = 0;

    // if more than one question, return error
    if (dns_message->header.qdcount != 1) {
        printf("\033[31m[_ERROR_]\033[0m QUERY %hu: more than one question in transaction\n",
               dns_message->header.identifier);
        response->header.flags.rcode = 1;
        return 1;
    }

    // if question is not of type A, return error
    if (dns_message->section.questions[0].type != A) {
        printf("\033[31m[_ERROR_]\033[0m QUERY %hu: question is not of type A\n", dns_message->header.identifier);
        response->header.flags.rcode = 4;
        return 1;
    }

    // if question is not of class IN, return error
    if (dns_message->section.questions[0].class != IN) {
        printf("\033[31m[_ERROR_]\033[0m QUERY %hu: question is not of class IN\n", dns_message->header.identifier);
        response->header.flags.rcode = 4;
        return 1;
    }

    // if inverse query, return error
    if (dns_message->header.flags.opcode == 1) {
        printf("\033[31m[_ERROR_]\033[0m QUERY %hu: inverse query not supported\n", dns_message->header.identifier);
        response->header.flags.rcode = 4;
        return 1;
    }
    return 0;
}


static int set_answer(dns_message_t *dns_message, dns_message_t *response, dns_resource_record_t *dns_resource_records,
                      size_t dns_resource_records_size) {
    char *domain_name = malloc(sizeof(char) * 256);
    domain_name[0] = '\0';
    for (int i = 0; i < dns_message->section.questions[0].labels_size; i += 1) {
        strcat(domain_name, dns_message->section.questions[0].labels[i].name);
        if (i != dns_message->section.questions[0].labels_size - 1) strcat(domain_name, ".");
    }

    printf("\033[34m[_INFO_]\033[0m QUERY %hu: looking for DOMAIN: %s\n",
           dns_message->header.identifier, domain_name);

    // find resource record
    dns_resource_record_t *dns_resource_record = NULL;
    for (int i = 0; i < dns_resource_records_size; i += 1) {
        if (strcmp(dns_resource_records[i].name, domain_name) == 0) {
            dns_resource_record = &dns_resource_records[i];
            break;
        }
    }

    // if resource record not found, return error
    if (dns_resource_record == NULL) {
        printf("\033[31m[_ERROR_]\033[0m QUERY %hu: resource record not found\n", dns_message->header.identifier);
        response->header.flags.rcode = 3;
        return 1;
    }


    // copy resource record
    response->section.answers = malloc(sizeof(dns_message_resource_t));
    response->section.answers[0].type = dns_resource_record->type;
    response->section.answers[0].class = IN;
    response->section.answers[0].ttl = 0;
    response->section.answers[0].rdlength = 4;
    response->section.answers[0].rdata = malloc(sizeof(uint32_t));
    memcpy(response->section.answers[0].rdata, &dns_resource_record->ip, sizeof(uint32_t));
    response->section.answers[0].name = malloc(sizeof(char) * 2);
    response->section.answers[0].name[0] = 12 * 16; // pointer to domain name
    response->section.answers[0].name[1] = 0xc;    // location of domain name

    return 0;
}

