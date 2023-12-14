#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "http_parser.h"


#define ERROR -1
#define OK 0


static int get_line(char *buffer, char **line, size_t len);

static int parse_request_line(http_request_t *hr, char *buffer, size_t buffer_size);

static bool is_request_line_available(const http_request_t *hr);

int http_parse_request(http_request_t *hr, char *buffer, size_t buffer_size) {

    // check if rq_line is available
    if (is_request_line_available(hr)) return OK;

    // parse request line
    int offset;
    if ((offset = parse_request_line(hr, buffer, buffer_size)) < 0) return ERROR;
    return offset;
}

static bool is_request_line_available(const http_request_t *hr) {
    return hr->request_line.method != 0 && hr->request_line.path.host != NULL && hr->request_line.version != 0.0;
}

static int parse_request_line(http_request_t *hr, char *buffer, size_t buffer_size) {

    char *request_line;
    size_t parsed_offset = get_line(buffer, &request_line, buffer_size);
    if (parsed_offset == EOF) return ERROR;

    // parse request line
    char *method = strtok(request_line, " ");
    char *uri = strtok(NULL, " ");
    char *version = strtok(NULL, " ");
    if (http_request_method_resolve(hr, method)) return 1;

    // set uri
    // if http:// exist in uri, remove it
    if (strstr(uri, "http://") != NULL) {
        uri += 7;
    }
    // if https:// exist in uri, remove it
    if (strstr(uri, "https://") != NULL) {
        uri += 8;
    }
    // get host, path and port
    char *host = strtok(uri, "/");
    char *path = strtok(NULL, "/");
    char *port = strtok(host, ":");
    port = strtok(NULL, ":");
    if (port == NULL) {
        port = "80";
    }

    hr->request_line.path.host = host;
    hr->request_line.path.path = path;
    hr->request_line.path.port = atoi(port);

    // set version
    // remove HTTP/ from version
    version += 5;
    hr->request_line.version = http_request_version_resolve(atof(version));

    return 0;
}


static int get_line(char *buffer, char **line, size_t len) {

    // read buffer until reaching \r\n
    int end_index = -1;
    for (int i = 0; i < len; i += 1) {
        if (buffer[i] == '\r' && buffer[i + 1] == '\n') {
            end_index = i;
            break;
        }
    }

    if (end_index == -1) {
        return EOF;
    }

    // copy the line to line
    *line = malloc(end_index + 1);
    memcpy(*line, buffer, end_index);
    (*line)[end_index] = '\0';

    return end_index;
}


int stringify_http_request(http_request_t *hr, char **buffer, size_t *buffer_size) {

}