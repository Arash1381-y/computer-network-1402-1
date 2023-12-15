#include "http_request.h"


ConstString http_version_to_string(float version)
{
    switch ((int) (version * 10)) {
        case 10:
            return "HTTP/1.0";
        case 11:
            return "HTTP/1.1";
        case 20:
            return "HTTP/2.0";
        default:
            return "UNKNOWN_VERSION";
    }
}


http_version_t http_request_version_resolve(float version)
{
    switch ((int) (version * 10)) {
        case 10:
            return HTTP_1_0;
        case 11:
            return HTTP_1_1;
        case 20:
            return HTTP_2_0;
        default:
            return UNKNOWN_VERSION;
    }
}

int http_request_method_resolve(http_request_t *hr, char *method)
{
    if (strcmp(method, "GET") == 0) {
        hr->request_line.method = GET;
    } else if (strcmp(method, "POST") == 0) {
        hr->request_line.method = POST;
    } else if (strcmp(method, "PUT") == 0) {
        hr->request_line.method = PUT;
    } else if (strcmp(method, "DELETE") == 0) {
        hr->request_line.method = DELETE;
    } else if (strcmp(method, "HEAD") == 0) {
        hr->request_line.method = HEAD;
    } else if (strcmp(method, "OPTIONS") == 0) {
        hr->request_line.method = OPTIONS;
    } else if (strcmp(method, "TRACE") == 0) {
        hr->request_line.method = TRACE;
    } else if (strcmp(method, "CONNECT") == 0) {
        hr->request_line.method = CONNECT;
    } else {
        return 1;
    }
    return 0;
}

ConstString http_request_method_to_string(enum http_request_method method)
{
    switch (method) {
        case GET:
            return "GET";
        case POST:
            return "POST";
        case PUT:
            return "PUT";
        case DELETE:
            return "DELETE";
        case HEAD:
            return "HEAD";
        case OPTIONS:
            return "OPTIONS";
        case TRACE:
            return "TRACE";
        case CONNECT:
            return "CONNECT";
        default:
            return "UNKNOWN_VERSION";
    }
}

bool http_request_line_is_valid(http_request_line_t *request_line)
{
    return request_line->method != UNKNOWN_METHOD && request_line->path.host != NULL &&
           request_line->path.path != NULL && request_line->path.port != 0 && request_line->version != UNKNOWN_VERSION;
}

int http_request_init(http_request_t *hr)
{
    hr->request_line.method = UNKNOWN_METHOD;
    hr->request_line.path.host = NULL;
    hr->request_line.path.path = NULL;
    hr->request_line.path.port = 0;
    hr->request_line.version = UNKNOWN_VERSION;
    hr->headers = NULL;
    hr->headers_size = 0;
    return 0;
}