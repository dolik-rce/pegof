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

template <typename... T>
void debug(const char* format, T... args) {
    if (Config::get<bool>("debug")) {
        DebugIndent::print();
        fprintf(stderr, format, args...);
        fprintf(stderr, "\n");
    }
}

template <typename... T>
void log(int level, const char* format, T... args) {
    if (Config::verbose(level)) {
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
    throw 1;
}
