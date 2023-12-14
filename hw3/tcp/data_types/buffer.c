#include "buffer.h"


int buff_init(buffer_t **buff, uint32_t size) {
    if (size == 0) {
        return -1;
    }

    *buff = malloc(sizeof(buffer_t));
    if (*buff == NULL) {
        return -1;
    }

    (*buff)->size = size;
    (*buff)->buff = malloc(size);
    (*buff)->current_index = 0;

    omp_init_lock(&(*buff)->lock);

    return 0;
}

char *buff_get_empty_addr(buffer_t *buff) {
    return buff->buff + buff->current_index;
}

int buff_get_empty_len(buffer_t *buff) {
    return buff->size - buff->current_index;
}

int buff_ge_filled_len(buffer_t *buff) {
    return buff->current_index;
}

int buff_fill(buffer_t *buff, int len) {
    buff->current_index += len;

    if (buff->current_index == (buff->size >> 1)) {
        omp_set_lock(&buff->lock);
        buff->size = buff->size * 2;
        buff->buff = realloc(buff->buff, buff->size);
        if (buff->buff == NULL) {
            return -1;
        }
        omp_unset_lock(&buff->lock);
    }

    return 1;
}