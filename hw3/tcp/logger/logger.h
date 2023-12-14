#ifndef TCP_LOGGER_H
#define TCP_LOGGER_H


#include <bits/types/FILE.h>
#include <stdint.h>
#include <malloc.h>


static const uint32_t DEFAULT_SUCCESS_COLOR = 0x00FF00;
static const uint32_t DEFAULT_ERROR_COLOR = 0xFF0000;


typedef struct logger {
    void *out;
    void *err;

    int32_t success_color;
    int32_t error_color;
    int32_t info_color;
} logger_t;

/**
 * @brief Initialize logger_t with given parameters.
 *
 *
 * @param l        Pointer to logger_t address.
 * @param out           Pointer to file which logs are written into
 * @param err           Pointer to file which errors are written into
 * @param success_color ANSI code which is used for success messages. if 0, default color is used.
 * @param error_color   ANSI code which is used for error messages. if 0, default color is used.
 * @param info_color    ANSI code which is used for info messages. if 0, default color is used.
 *
 * @return              0 if success, 1 if error.
 */
int logger_init(
        logger_t **l,
        void *out,
        void *err,

        int32_t success_color,
        int32_t error_color,
        int32_t info_color
);


void success(logger_t *l, const char *format, ...);

void error(logger_t *l, const char *format, ...);

void info(logger_t *l, const char *format, ...);

#endif //TCP_LOGGER_H
