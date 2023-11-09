#include <stddef.h>
#include <malloc.h>
#include <string.h>

#include "data-types/dns_message_t.h"

#define BYTE 8

// ============================ HELPER FUNCTIONS========================================
static void extract_additional(dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc);

static size_t extract_authorities(const dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc);

static size_t extract_answers(dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc);

static int extract_labels(const dns_message_t *dns_message, const unsigned char *buffer, size_t buffer_size,
                          uint8_t allocated_labels, size_t *s_loc, uint8_t *labels, dns_message_label_t **labels_array);

static void extract_flags(dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc);

static void extract_counts(dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc);
// =====================================================================================

/**
 * @brief parse a dns message from a buffer and store it in `dns_message`
 * @param dns_message  pointer to dns_message_t
 * @param buffer  pointer to buffer which contains the dns message
 * @param buffer_size  size of the buffer
 * @return  0 if success, 1 if error
 */
int parse_dns_message(dns_message_t *dns_message, unsigned char *buffer, size_t buffer_size) {
    size_t s_loc = 0;

    // extract identifier
    if (s_loc + 2 > buffer_size) return 1;
    uint16_t identifier = buffer[s_loc] << BYTE | buffer[s_loc + 1];
    dns_message->header.identifier = identifier;
    s_loc += 2;

    // extract flags
    if (s_loc + 2 > buffer_size) return 1;
    extract_flags(dns_message, buffer, s_loc);
    s_loc += 2;

    // if not a query do not parse the rest of the message
    if (dns_message->header.flags.qr == 1) {
        printf("\033[34m[_INFO_]\033[0m received a response message with identifier : %d\n",
               dns_message->header.identifier);
    }

    // extract counts
    if (s_loc + 2 * 4 > buffer_size) return 1;
    extract_counts(dns_message, buffer, s_loc);
    s_loc += 2 * 4;

    // extract questions
    if (s_loc + dns_message->header.qdcount * 4 > buffer_size) return 1;
    dns_message->section.questions = malloc(sizeof(dns_message_question_t) * dns_message->header.qdcount);
    for (int i = 0; i < dns_message->header.qdcount; i += 1) {
        uint8_t labels = 0;
        uint8_t allocated_labels = 10;

        dns_message_label_t *labels_array = malloc(sizeof(dns_message_label_t) * allocated_labels);

        if (extract_labels(dns_message, buffer, buffer_size, allocated_labels, &s_loc, &labels, &labels_array))
            return 1;

        dns_message->section.questions[i].labels = labels_array;
        dns_message->section.questions[i].labels_size = labels;
        dns_message->section.questions[i].type = buffer[s_loc] << BYTE | buffer[s_loc + 1];
        dns_message->section.questions[i].class = buffer[s_loc + 2] << BYTE | buffer[s_loc + 3];
        s_loc += 4;
    }

    // extract answers
    if (s_loc + dns_message->header.ancount * 10 > buffer_size) return 1;
    s_loc = extract_answers(dns_message, buffer, s_loc);

    // extract authorities
    if (s_loc + dns_message->header.nscount * 10 > buffer_size) return 1;
    dns_message->section.authorities = malloc(sizeof(dns_message_resource_t) * dns_message->header.nscount);
    s_loc = extract_authorities(dns_message, buffer, s_loc);

    // extract additional
    if (s_loc + dns_message->header.arcount * 10 > buffer_size) return 1;
    extract_additional(dns_message, buffer, s_loc);

    return 0;
}

/**
 * @brief  set qdcount of dns message
 * @param dns_message
 * @param buffer
 * @param s_loc
 */
static void extract_counts(dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc) {
    dns_message->header.qdcount = buffer[s_loc] << BYTE | buffer[s_loc + 1];
    dns_message->header.ancount = buffer[s_loc + 2] << BYTE | buffer[s_loc + 3];
    dns_message->header.nscount = buffer[s_loc + 4] << BYTE | buffer[s_loc + 5];
    dns_message->header.arcount = buffer[s_loc + 6] << BYTE | buffer[s_loc + 7];
}

/**
 * @brief set flags of dns message
 * @param dns_message
 * @param buffer
 * @param s_loc
 */
static void extract_flags(dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc) {
    dns_message->header.flags.qr = buffer[s_loc] >> 7;
    dns_message->header.flags.opcode = buffer[s_loc] >> 3 & 0xf;
    dns_message->header.flags.aa = buffer[s_loc] >> 2 & 0x1;
    dns_message->header.flags.tc = buffer[s_loc] >> 1 & 0x1;
    dns_message->header.flags.rd = buffer[s_loc] & 0x1;
    dns_message->header.flags.ra = buffer[s_loc + 1] >> 7;
    dns_message->header.flags.z = buffer[s_loc + 1] >> 4 & 0x7;
    dns_message->header.flags.rcode = buffer[s_loc + 1] & 0xf;
}

/**
 * @brief extract labels from buffer and store them in `labels_array`
 * @param dns_message
 * @param buffer
 * @param buffer_size
 * @param allocated_labels
 * @param s_loc
 * @param labels
 * @param labels_array
 * @return
 */
static int extract_labels(const dns_message_t *dns_message, const unsigned char *buffer, size_t buffer_size,
                          uint8_t allocated_labels, size_t *s_loc, uint8_t *labels,
                          dns_message_label_t **labels_array) {
    while (1) {
        // read the first byte (len of label)
        uint8_t len = buffer[(*s_loc)];
        if (len == 0) break;

        // read the label
        char *name = malloc(sizeof(char) * (len + 1));
        memcpy(name, buffer + (*s_loc) + 1, len);
        name[len] = '\0';

        // create a label and append it to the questions.labels
        dns_message_label_t label;
        label.name = name;
        label.size = len;

        if ((*labels) == allocated_labels) {
            allocated_labels *= 2;
            (*labels_array) = realloc((*labels_array), sizeof(dns_message_label_t) * allocated_labels);

            if ((*labels_array) == NULL) {
                printf("\033[31m[_ERROR_]\033[0m QUERY %hu: can not allocate memory for labels\n",
                       dns_message->header.identifier);
                return 1;
            }
        }
        (*labels_array)[(*labels)] = label;
        (*labels) += 1;
        (*s_loc) += len + 1;

        if ((*s_loc) > buffer_size) return 1;
    }
    *s_loc += 1;
    return 0;
}

/**
 * @brief extract answers from buffer and store them in `dns_message`
 * @param dns_message
 * @param buffer
 * @param s_loc
 * @return
 */
static size_t extract_answers(dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc) {
    dns_message->section.answers = malloc(sizeof(dns_message_resource_t) * dns_message->header.ancount);
    for (int i = 0; i < dns_message->header.ancount; i += 1) {
        dns_message->section.answers[i].name = (unsigned char *) buffer + s_loc;
        s_loc += strlen(dns_message->section.answers[i].name) + 1;
        dns_message->section.answers[i].type = buffer[s_loc] << BYTE | buffer[s_loc + 1];
        dns_message->section.answers[i].class = buffer[s_loc + 2] << BYTE | buffer[s_loc + 3];
        dns_message->section.answers[i].ttl =
                buffer[s_loc + 4] << BYTE * 3 | buffer[s_loc + 5] << BYTE * 2 | buffer[s_loc + 6] << BYTE |
                buffer[s_loc + 7];
        dns_message->section.answers[i].rdlength = buffer[s_loc + 8] << BYTE | buffer[s_loc + 9];
        dns_message->section.answers[i].rdata = buffer + s_loc + 10;
        s_loc += 10 + dns_message->section.answers[i].rdlength;
    }
    return s_loc;
}

/**
 * @brief extract authorities from buffer and store them in `dns_message`
 * @param dns_message
 * @param buffer
 * @param s_loc
 * @return
 */
static size_t extract_authorities(const dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc) {
    for (int i = 0; i < dns_message->header.nscount; i += 1) {
        dns_message->section.authorities[i].name = (unsigned char *) buffer + s_loc;
        s_loc += strlen(dns_message->section.authorities[i].name) + 1;
        dns_message->section.authorities[i].type = buffer[s_loc] << BYTE | buffer[s_loc + 1];
        dns_message->section.authorities[i].class = buffer[s_loc + 2] << BYTE | buffer[s_loc + 3];
        dns_message->section.authorities[i].ttl =
                buffer[s_loc + 4] << BYTE * 3 | buffer[s_loc + 5] << BYTE * 2 | buffer[s_loc + 6] << BYTE |
                buffer[s_loc + 7];
        dns_message->section.authorities[i].rdlength = buffer[s_loc + 8] << BYTE | buffer[s_loc + 9];
        dns_message->section.authorities[i].rdata = buffer + s_loc + 10;
        s_loc += 10 + dns_message->section.authorities[i].rdlength;
    }
    return s_loc;
}

void extract_additional(dns_message_t *dns_message, const unsigned char *buffer, size_t s_loc) {
    dns_message->section.additional = malloc(sizeof(dns_message_resource_t) * dns_message->header.arcount);
    for (int i = 0; i < dns_message->header.arcount; i += 1) {
        dns_message->section.additional[i].name = (unsigned char *) buffer + s_loc;
        s_loc += strlen(dns_message->section.additional[i].name) + 1;
        dns_message->section.additional[i].type = buffer[s_loc] << BYTE | buffer[s_loc + 1];
        dns_message->section.additional[i].class = buffer[s_loc + 2] << BYTE | buffer[s_loc + 3];
        dns_message->section.additional[i].ttl =
                buffer[s_loc + 4] << BYTE * 3 | buffer[s_loc + 5] << BYTE * 2 | buffer[s_loc + 6] << BYTE |
                buffer[s_loc + 7];
        dns_message->section.additional[i].rdlength = buffer[s_loc + 8] << BYTE | buffer[s_loc + 9];
        dns_message->section.additional[i].rdata = buffer + s_loc + 10;
        s_loc += 10 + dns_message->section.additional[i].rdlength;
    }
}
