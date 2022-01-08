#ifndef INCLUDED_SOURCE_H
#define INCLUDED_SOURCE_H

#include <string>
#include <utility>
#include <cstdio>

class Io {
    static std::string text;
    static size_t pos;
    static FILE *output;
    static bool verbose;
public:
    static void set_verbose(const bool enabled);
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
        if (verbose) {
            fprintf(stderr, format, args...);
        }
    }
};

#endif
