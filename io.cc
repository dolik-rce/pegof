#include "io.h"

#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

void Io::open(const string& path) {
    close();
    int fd = ::open(path.c_str(), O_RDONLY);
    struct stat s;
    int status = fstat (fd, &s);
    filesize = s.st_size;
    mapped = mmap (0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    pos = 0;
    is_open = true;
    ::close(fd);
    text = string_view(reinterpret_cast<const char*>(mapped));
}

void Io::close() {
    if (is_open) {
        munmap(mapped, filesize);
        is_open = false;
    }
}

int Io::read() {
    if (pos < text.size()) {
        return text[pos++];
    } else {
        return EOF;
    }
}

std::pair<size_t, size_t> Io::compute_position(size_t start) const {
    size_t line = std::count(text.begin(), text.begin() + start, '\n') + 1;
    size_t col = start - text.rfind('\n', start);
    return {line, col};
}

Io::Io() : is_open(false) {}

Io::~Io() {
    if (is_open) {
        close();
    }
}
