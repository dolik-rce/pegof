#ifndef INCLUDED_SOURCE_H
#define INCLUDED_SOURCE_H

#include <string>
#include <utility>

using std::string;
using std::string_view;

class Io {
    string path;
    size_t filesize;
    void *mapped;
    string_view text;
    size_t pos;
    bool is_open;

    void close();

public:
    void open(const string& path);
    int read();
    std::pair<size_t, size_t> compute_position(size_t start) const;

    Io();
    ~Io();
};

#endif
