#ifndef TCP_HTTP_REQUEST_H
#define TCP_HTTP_REQUEST_H


#include <stddef.h>


typedef enum http_request_method {
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    OPTIONS,
    TRACE,
    CONNECT
} http_request_method_t;

char *http_request_method_to_string(http_request_method_t method);


typedef enum http_version {
    UNKNOWN = 00,
    HTTP_1_0 = 10,
    HTTP_1_1 = 11,
    HTTP_2_0 = 20
} http_version_t;

http_version_t http_request_version_resolve(float version);

char *http_version_to_string(float version);


typedef struct {
    char *host;
    char *path;
    int port;
} http_request_url_t;

typedef struct {
    http_request_method_t method;
    http_request_url_t path;
    float version;
} http_request_line_t;

typedef struct {
    char *key;
    char *value;
} http_header_t;


typedef struct {
    http_request_line_t request_line;
    http_header_t *headers;
    size_t headers_size;
} http_request_t;

int http_request_method_resolve(http_request_t *hr, char *method);

#endif //TCP_HTTP_REQUEST_H
