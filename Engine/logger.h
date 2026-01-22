/**
 * @file logger.h
 * @brief Centralized logging system for Pallas Chess Engine
 * 
 * This header provides a unified logging interface that replaces the old DEBUG_LOG macros.
 * Uses log.c library by rxi for flexible, level-based logging.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "vendor/log.h"

// Convenience macros that match the old DEBUG_LOG pattern
#define LOG_TRACE(...) log_trace(__VA_ARGS__)
#define LOG_DEBUG(...) log_debug(__VA_ARGS__)
#define LOG_INFO(...)  log_info(__VA_ARGS__)
#define LOG_WARN(...)  log_warn(__VA_ARGS__)
#define LOG_ERROR(...) log_error(__VA_ARGS__)
#define LOG_FATAL(...) log_fatal(__VA_ARGS__)

// Initialize the logging system
void logger_init(void);

// Set the minimum log level (LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL)
void logger_set_level(int level);

// Enable/disable quiet mode (suppress output to stderr)
void logger_set_quiet(bool quiet);

// Add a log file output
void logger_add_file(const char *path, int level);

#endif // LOGGER_H
