#include "io.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

std::string Io::text;
size_t Io::pos = 0;
FILE *Io::output = stdin;
bool Io::verbose = false;

void Io::set_verbose(const bool enabled) {
    verbose = enabled;
}


void Io::open(const std::string& input_path, const std::string& output_path) {
    std::stringstream stream;
    if (input_path.empty()) {
        Io::debug("Reading grammar from stdin\n");
        stream << std::cin.rdbuf();
    } else {
        Io::debug("Reading grammar from %s\n", input_path.c_str());
        std::ifstream input(input_path);
        stream << input.rdbuf();
    }

    text = stream.str();
    pos = 0;
    output = output_path.empty() ? stdout : fopen(output_path.c_str(), "w");
    Io::debug("Output will be written to %s\n", output_path.empty() ? "stdout" : output_path.c_str());
}

void Io::close() {
    if (output != stdout) {
        fclose(output);
        output = NULL;
    }
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
