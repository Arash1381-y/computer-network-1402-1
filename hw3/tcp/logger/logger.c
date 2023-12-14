#include <stdarg.h>
#include "logger.h"


int logger_init(
        logger_t **l,

        void *out,
        void *err,

        int32_t success_color,
        int32_t error_color,
        int32_t info_color
) {

    *l = malloc(sizeof(logger_t));
    if (*l == NULL) {
        return -1;
    }

    (*l)->out = out;
    (*l)->err = err;

    (*l)->success_color = success_color;
    (*l)->error_color = error_color;
    (*l)->info_color = info_color;

    return 0;
}


void success(logger_t *l, const char *format, ...) {
    int red = (l->success_color >> 16) & 0xFF;
    int green = (l->success_color >> 8) & 0xFF;
    int blue = l->success_color & 0xFF;

    // Print the color and success message format
    printf("\033[38;2;%d;%d;%dm[_SUCCESS_]\033[0m ", red, green, blue);

    // Use va_list and vprintf to handle variable arguments
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

void error(logger_t *l, const char *format, ...) {
    int red = (l->error_color >> 16) & 0xFF;
    int green = (l->error_color >> 8) & 0xFF;
    int blue = l->error_color & 0xFF;

    // Print the color and success message format
    printf("\033[38;2;%d;%d;%dm[_ERROR_]\033[0m ", red, green, blue);

    // Use va_list and vprintf to handle variable arguments
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

void info(logger_t *l, const char *format, ...) {
    int red = (l->info_color >> 16) & 0xFF;
    int green = (l->info_color >> 8) & 0xFF;
    int blue = l->info_color & 0xFF;

    // Print the color and success message format
    printf("\033[38;2;%d;%d;%dm[_INFO_]\033[0m ", red, green, blue);

    // Use va_list and vprintf to handle variable arguments
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

void logger_destroy(logger_t *l) {
    free(l);
}