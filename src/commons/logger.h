#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_ERROR
} LogLevel;

void log_message(LogLevel level, const char *file, int line, const char *format, ...);

#define LOG_INFO(...) log_message(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif
