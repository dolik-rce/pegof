#pragma once
#include "config.h"

#include <stdio.h>

class DebugIndent {
    static int indent(int increment = 0);

public:
    DebugIndent();
    ~DebugIndent();
    static void inc();
    static void dec();
    static void print();
};

enum ExitCode { INVALID_ARG = 1, IO_ERROR = 2, SCRIPT_ERROR = 3, INTERNAL_ERROR = 5, PARSING_ERROR = 10 };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"

template<typename... T> void debug(const char* format, T... args) {
    if (Config::get<bool>("debug")) {
        DebugIndent::print();
        fprintf(stderr, format, args...);
        fprintf(stderr, "\n");
    }
}

template<typename... T> void log(int level, const char* format, T... args) {
    if (Config::verbose(level)) {
        fprintf(stderr, format, args...);
        fprintf(stderr, "\n");
    }
}

template<typename... T> void warn(const char* format, T... args) {
    fprintf(stderr, "WARNING: ");
    fprintf(stderr, format, args...);
    fprintf(stderr, "\n");
}

template<typename... T> void error [[noreturn]] (ExitCode exit_code, const char* format, T... args) {
    fprintf(stderr, "ERROR: ");
    fprintf(stderr, format, args...);
    fprintf(stderr, "\n");
    throw (int)exit_code;
}

#pragma GCC diagnostic pop
