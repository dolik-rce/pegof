#ifndef INCLUDED_SOURCE_H
#define INCLUDED_SOURCE_H

#include <string>
#include <utility>

using std::string;
using std::string_view;

class Source {
    string path;
    size_t filesize;
    void *mapped;
    string_view text;
    size_t pos;

    void open();
    void close();

public:
    int read();
    std::pair<size_t, size_t> computePosition(size_t start);

    Source(const char* filepath) : path(filepath), pos(0) {
        open();
    }

    ~Source() {
        close();
    }
};

#endif
