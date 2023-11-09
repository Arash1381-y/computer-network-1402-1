#include "dns_loader.h"


// ============================ HELPER FUNCTIONS========================================
static int extract_IPV4(char *line, char **ip, char **domain);
// =====================================================================================


/**
 * @brief load DNS server data from file into `dns_resource_records` and set `dns_resource_records_size`
 * @param path  path to RR source file
 * @param dns_resource_records  pointer to dns_resource_records
 * @param dns_resource_records_size  pointer to dns_resource_records_size
 * @return
 */
int load_dns_server_data(const char *path, dns_resource_record_t **dns_resource_records,
                         size_t *dns_resource_records_size) {


    // configure RRs source file path
    if (path == NULL) {
        path = DEFAULT_DNS_SERVERS_PATH;
        printf("\033[33m[_WARNING_]\033[0m path is NULL, using default path: %s\n", DEFAULT_DNS_SERVERS_PATH);
    }


    // open RRs source file
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("\033[31m[_ERROR_]\033[0m can not open file: %s\n", path);
        return 1;
    }


    // read RRs source file line by line
    char *line = NULL;
    size_t len = 0, n_lines = 0;
    ssize_t read;
    while ((read = getline(&line, &len, file)) != -1) {
        // if blank line
        if (read == 1) continue;



        // extract IPv4 from line and append it to `dns_resource_records`
        char *ip;
        char *domain;


        if (!extract_IPV4(line, &ip, &domain)) {


            if (n_lines == *dns_resource_records_size) {
                // allocate memory for new RR

                *dns_resource_records = realloc(*dns_resource_records, sizeof(dns_resource_record_t) * (n_lines * 2));
                *dns_resource_records_size = n_lines * 2;

                if (*dns_resource_records == NULL) {
                    printf("\033[31m[_ERROR_]\033[0m can not allocate memory for DNS server data\n");
                    return 1;
                }
            }
            // set RR
            (*dns_resource_records)[n_lines].ip = inet_addr(ip);
            (*dns_resource_records)[n_lines].name = malloc(sizeof(char) * (strlen(domain) + 1));
            strncpy((*dns_resource_records)[n_lines].name, domain, strlen(domain));
            (*dns_resource_records)[n_lines].name[strlen(domain)] = '\0';
            (*dns_resource_records)[n_lines].type = A;
            n_lines += 1;
        }
    }

    *dns_resource_records_size = n_lines;

    // close RRs source file
    fclose(file);
    if (line) free(line);

    printf("\033[34m[_INFO_]\033[0m loaded %zu DNS server data from %s\n", n_lines, path);
    return 0;
}

/**
 * @brief extract IPv4 from line and append it to `dns_resource_records`
 * @param dns_resource_records
 * @param dns_resource_records_size
 * @param line
 * @param n_lines
 */
static int extract_IPV4(char *line, char **ip, char **domain) {
    // split IP and domain name
    char *ip_extracted = strtok(line, " ");
    char *domain_name = strtok(NULL, " ");
    domain_name[strlen(domain_name) - 1] = '\0';

    if (ip_extracted == NULL || domain_name == NULL) return 1;

    *ip = ip_extracted;
    *domain = domain_name;

    return 0;

}
