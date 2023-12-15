#ifndef TCP_HTTP_REQUEST_H
#define TCP_HTTP_REQUEST_H

#include <string.h>
#include <stdbool.h>
#include <stddef.h>


#include "CbNB.h"


typedef enum http_request_method {
    UNKNOWN_METHOD = 0, GET, POST, PUT, DELETE, HEAD, OPTIONS, TRACE, CONNECT
} http_request_method_t;

/**
 * @brief Convert a string to a http_request_method_t
 *
 * @param method  string to convert
 *
 * @return        http_request_method_t
 */
ConstString http_request_method_to_string(http_request_method_t method);


typedef enum http_version {
    UNKNOWN_VERSION = 00, HTTP_1_0 = 10, HTTP_1_1 = 11, HTTP_2_0 = 20
} http_version_t;


/**
 * @brief Convert a float to a http_version_t
 *
 * @param version  float to convert
 *
 * @return         http_version_t
 */
http_version_t http_request_version_resolve(float version);

/**
 * @brief Convert a http_version_t to a string
 *
 * @param version  http_version_t to convert
 *
 * @return         string representation of http_version_t
 */
ConstString http_version_to_string(float version);


typedef struct {
    char *host;
    char *path;
    int port;
} http_request_url_t;

typedef struct {
    http_request_method_t method;
    http_request_url_t path;
    http_version_t version;
} http_request_line_t;

/**
 *  @brief Check if a http_request_line_t is valid by checking if the method, path and version are valid
 *
 * @param request_line   http_request_line_t to check
 *
 * @return              true if valid otherwise false
 */
bool http_request_line_is_valid(http_request_line_t *request_line);

typedef struct {
    char *key;
    char *value;
} http_header_t;


typedef struct {
    http_request_line_t request_line;
    http_header_t *headers;
    size_t headers_size;
} http_request_t;


/**
 * @brief Parse a http request from a buffer
 *
 * @param hr    http_request_t to fill
 *
 * @return      1 if error otherwise 0
 */
int http_request_init(http_request_t *hr);

/**
 * @brief Parse a http request from a buffer
 *
 * @param hr      http_request_t to fill
 * @param method  string to convert
 * @return        1 if error otherwise 0
 */
int http_request_method_resolve(http_request_t *hr, char *method);

#endif //TCP_HTTP_REQUEST_H
