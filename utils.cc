#include "utils.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>

std::string read_file(const std::string& filename) {
    std::stringstream buffer;
    buffer << (filename.empty() ? std::cin.rdbuf() : std::ifstream(filename).rdbuf());
    return buffer.str();
}

void write_file(const std::string& filename, const std::string& content) {
    if (filename.empty()) {
        printf("%s", content.c_str());
    } else {
        std::ofstream ofs(filename);
        ofs << content;
        ofs.close();
    }
}

std::string to_c_string(std::string str, EscapeMode mode) {
    std::string result;
    for (int i = 0; i < str.size(); i++) {
        switch (str[i]) {
        case '\x00': result += "\\0"; break;
        case '\x07': result += "\\a"; break;
        case '\x08': result += "\\b"; break;
        case '\x0c': result += "\\f"; break;
        case '\x0a': result += "\\n"; break;
        case '\x0d': result += "\\r"; break;
        case '\x09': result += "\\t"; break;
        case '\x0b': result += "\\v"; break;
        case '\\':   result += "\\\\"; break;
        case '\'':   result += (mode & ESCAPE_SINGLE_QUOTES) ? "\\'" : "'"; break;
        case '\"':   result += (mode & ESCAPE_DOUBLE_QUOTES) ? "\\\"" : "\""; break;
        default:
            if (str[i] >= '\x20' && str[i] < '\x7f') {
                result += str[i];
            } else {
                std::stringstream stream;
                stream << std::setfill ('0') << std::setw(2) << std::hex << (int)(unsigned char)str[i];
                result += "\\x" + stream.str();
            }
        }
    }
    return result;
}

std::string trim(const std::string& str, TrimType type) {
    size_t start = (type & TRIM_LEFT) ? str.find_first_not_of(" \t\r\n") : 0;
    size_t end = (type & TRIM_RIGHT) ? str.find_last_not_of(" \t\r\n") + 1 : str.size();
    if (start == std::string::npos) {
        start = 0;
    }
    return std::string(str.c_str() + start, end - start);
}

std::vector<std::string> split(const std::string &s, const std::string& delimiter) {
    std::regex sep_regex(delimiter);
    std::sregex_token_iterator iter(s.begin(), s.end(), sep_regex, -1);
    std::sregex_token_iterator end;
    return {iter, end};
}

bool contains(std::vector<std::string> values, std::string x) {
    for (std::string& v: values) {
        if (v == x) return true;
    }
    return false;
}

std::string replace(const std::string& input, const std::string& re, const std::string& replace) {
    return std::regex_replace(input, std::regex(re), replace);
}

std::string left_pad(const std::string& s, int width) {
    return std::string(width - s.size(), ' ') + s;
}
