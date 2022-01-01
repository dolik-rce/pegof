#ifndef INCLUDED_SOURCE_H
#define INCLUDED_SOURCE_H

#include <string>
#include <utility>

class Io {
    static std::string text;
    static size_t pos;
public:
    static void open(const std::string& path);
    static int read();
    static std::pair<size_t, size_t> compute_position(size_t start);
};

#endif
