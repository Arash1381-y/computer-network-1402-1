#include "CbNB.h"

void cbnb_log(CBNB_LOG_LEVEL level, ConstString format, ...)
{
    switch (level) {
        case DEBUG:
            printf(CBNB_LOGGER_COLOR_BLUE
                   "[DEBUG] "  COLOR_END);
            break;
        case INFO:
            printf(CBNB_LOGGER_COLOR_BLUE
                   "[INFO] " COLOR_END);
            break;
        case WARN:
            printf(CBNB_LOGGER_COLOR_YELLOW
                   "[WARN] " COLOR_END);
            break;
        case ERROR:
            printf(CBNB_LOGGER_COLOR_RED
                   "[ERROR] " COLOR_END);
            break;
        case FATAL:
            printf(CBNB_LOGGER_COLOR_RED
                   "[FATAL] ");
            break;
        default:
            printf("[UNKNOWN] ");
            break;
    }


    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}
