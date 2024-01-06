#ifndef CBNB_CBNB_H
#define CBNB_CBNB_H



// INCLUDES
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>



// ============================ FUNCTIONS ================================================
#define NOT_IMPLEMENTED return 0; // Not working for void functions

// ============================ STRINGS ==================================================
#define ConstString const char *

// =======================================================================================



// =========================== TIME ======================================================
#define Sec long long int
// =======================================================================================



// ============================ LOGGER ===================================================
#define CBNB_LOGGER_COLOR_RED     "\x1b[31m"
#define CBNB_LOGGER_COLOR_GREEN   "\x1b[32m"
#define CBNB_LOGGER_COLOR_YELLOW  "\x1b[33m"
#define CBNB_LOGGER_COLOR_BLUE    "\x1b[34m"
#define COLOR_END  "\x1b[0m"


typedef enum {
    DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3, FATAL = 4, NONE = 5
} CBNB_LOG_LEVEL;

void cbnb_log(CBNB_LOG_LEVEL level, ConstString format, ...);

#define CBNB_LOG(level, format, ...) cbnb_log(level, format, ##__VA_ARGS__)
// ======================================================================================


// ============================ FILE DESCRIPTOR =========================================
#define Fd int
#define Port int32_t

#define CBNB_Fd_CLOSE(fd) close(fd)
#define CBNB_SOCK_AVAILABLE(sfd) (fcntl(sfd, F_GETFD) != -1 || errno != EBADF)

// ======================================================================================


#endif //CBNB_CBNB_H

