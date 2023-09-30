#include "config.h"
#include <stdio.h>

template <typename... T>
void debug(const char* format, T... args) {
    if (Config::get<bool>("debug")) {
        fprintf(stderr, format, args...);
        fprintf(stderr, "\n");
    }
}

template <typename... T>
void log(int level, const char* format, T... args) {
    if (Config::verbose(level) || Config::get<bool>("debug")) {
        fprintf(stderr, format, args...);
        fprintf(stderr, "\n");
    }
}

template <typename... T>
void warn(const char* format, T... args) {
    fprintf(stderr, "WARNING: ");
    fprintf(stderr, format, args...);
    fprintf(stderr, "\n");
}

template <typename... T>
void error [[noreturn]] (const char* format, T... args) {
    fprintf(stderr, "ERROR: ");
    fprintf(stderr, format, args...);
    fprintf(stderr, "\n");
    exit(1);
}
