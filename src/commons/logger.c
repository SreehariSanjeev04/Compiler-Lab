#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void log_message(LogLevel level, const char *file, int line, const char *format, ...) {
    FILE *stream = level == LOG_LEVEL_ERROR ? stderr : stdout;
    const char *name = level == LOG_LEVEL_ERROR ? "ERROR" : "INFO";
    int color = isatty(fileno(stream));

    if (color)
        fputs(level == LOG_LEVEL_ERROR ? "\033[31m" : "\033[32m", stream);

    fprintf(stream, "[%s] %s:%d: ", name, file, line);
    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);

    size_t length = strlen(format);
    if (length == 0 || format[length - 1] != '\n')
        fputc('\n', stream);
    if (color)
        fputs("\033[0m", stream);
}
