#include "http_parser.h"



// ============================ HELPER FUNCTIONS ========================================
/**
 * @brief get next line from a buffer (reaching \r\n)
 *
 * @param buffer input buffer
 * @param line   dynamic array to save the line
 * @param len    length of the line (in char/bytes)
 *
 * @return       read bytes from buffer (index of first character after /r/n)
 */
static int get_line(char *buffer, char **line, size_t len);

/**
 * @brief parse request line by reading buffer and save it into http_request struct.
 * @param hr            http_request_t struct that save the request line
 * @param buffer        buffer that data is parsed from it
 * @param buffer_size   the size of buffer
 *
 * @return if request line is resolved return 0 otherwise -1(indicating an error)
 */
static int parse_request_line(http_request_t *hr, char *buffer, size_t buffer_size);
// ======================================================================================


int http_parse_request(http_request_t *hr, char *buffer, size_t buffer_size)
{

    // check if rq_line is available
    if (http_request_line_is_valid(&hr->request_line)) return OK;

    // parse request line
    int offset;
    if ((offset = parse_request_line(hr, buffer, buffer_size)) < 0) return -1;

    //TODO: parse headers if available and save it into a cache
    return offset;
}

static int parse_request_line(http_request_t *hr, char *buffer, size_t buffer_size)
{

    char *request_line;
    size_t parsed_offset = get_line(buffer, &request_line, buffer_size);
    if (parsed_offset == EOF) return -1;

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


static int get_line(char *buffer, char **line, size_t len)
{

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

    return end_index + 1;
}


int stringify_http_request(http_request_t *hr, char **buffer, size_t *buffer_size)
{

}