#ifndef INCLUDED_SOURCE_H
#define INCLUDED_SOURCE_H

#include "config.h"

#include <string>
#include <utility>
#include <cstdio>

class Io {
    static std::string text;
    static size_t pos;
    static FILE *output;
public:
    static void open(const std::string& input_path, const std::string& output_path);
    static void close();
    static int read();
    static std::pair<size_t, size_t> compute_position(size_t start);

    template <typename... T>
    static void print(const char* format, T... args) {
        fprintf(output, format, args...);
    }

    template <typename... T>
    static void debug(const char* format, T... args) {
        if (Config::get<bool>("verbose")) {
            fprintf(stderr, format, args...);
        }
    }
};

#endif
