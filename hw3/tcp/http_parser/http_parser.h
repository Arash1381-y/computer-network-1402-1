#ifndef TCP_HTTP_PARSER_H
#define TCP_HTTP_PARSER_H

#include "../data_types/http_request.h"


/**
 * @brief  Parse HTTP request from buffer
 *
 * @param hr           HTTP request structure
 * @param buffer       Buffer with HTTP request
 * @param buffer_size  Size of buffer
 * @return
 */
int http_parse_request(http_request_t *hr, char *buffer, size_t buffer_size);

int stringify_http_request(http_request_t *hr, char **buffer, size_t *buffer_size);

#endif //TCP_HTTP_PARSER_H
