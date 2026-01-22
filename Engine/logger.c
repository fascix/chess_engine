/**
 * @file logger.c
 * @brief Implementation of the centralized logging system
 */

#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

static FILE *log_file = NULL;

void logger_init(void) {
    // Set default log level based on DEBUG macro
    #ifdef DEBUG
        log_set_level(LOG_DEBUG);
    #else
        log_set_level(LOG_INFO);
    #endif
    
    // In release mode, we might want to be quiet on stderr
    #ifdef NDEBUG
        log_set_quiet(false); // Still show logs by default
    #endif
}

void logger_set_level(int level) {
    log_set_level(level);
}

void logger_set_quiet(bool quiet) {
    log_set_quiet(quiet);
}

void logger_add_file(const char *path, int level) {
    if (log_file != NULL) {
        fclose(log_file);
    }
    
    log_file = fopen(path, "a");
    if (log_file != NULL) {
        log_add_fp(log_file, level);
    }
}
