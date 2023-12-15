#ifndef TCP_HTTP_PARSER_H
#define TCP_HTTP_PARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http_request.h"

#define ERROR -1
#define OK 0


/**
 * @brief  Parse HTTP request from buffer
 *
 * @param hr           HTTP request structure
 * @param buffer       Buffer with HTTP request
 * @param buffer_size  Size of buffer
 *
 * @return 0 request_line is available. offset of parsed buffer if parsed successfully and -1 if encounters any error
 */
int http_parse_request(http_request_t *hr, char *buffer, size_t buffer_size);

//
int stringify_http_request(http_request_t *hr, char **buffer, size_t *buffer_size);

#endif //TCP_HTTP_PARSER_H
