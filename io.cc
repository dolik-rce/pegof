#include "io.h"

#include <algorithm>
#include <fstream>
#include <sstream>

std::string Io::text;
size_t Io::pos = 0;

void Io::open(const std::string& path) {
    std::ifstream input;
    input.open(path);
    std::stringstream stream;
    stream << input.rdbuf();

    text = stream.str();
    pos = 0;
}

int Io::read() {
    if (pos < text.size()) {
        return text[pos++];
    } else {
        return EOF;
    }
}

std::pair<size_t, size_t> Io::compute_position(size_t start) {
    size_t line = std::count(text.begin(), text.begin() + start, '\n') + 1;
    size_t col = start - text.rfind('\n', start);
    return {line, col};
}
