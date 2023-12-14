#ifndef TCP_BUFFER_H
#define TCP_BUFFER_H

#include <malloc.h>
#include <stdint.h>
#include <omp.h>

typedef struct tcp_buffer {
    uint32_t size;
    char *buff;
    uint32_t current_index;
    omp_lock_t lock;

} buffer_t;

int buff_init(buffer_t **buff, uint32_t size);

char *buff_get_empty_addr(buffer_t *buff);

int buff_get_empty_len(buffer_t *buff);

int buff_ge_filled_len(buffer_t *buff);

int buff_fill(buffer_t *buff, int len);


#endif //TCP_BUFFER_H
