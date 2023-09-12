#include "utils.h"

#include <fstream>
#include <sstream>
#include <iomanip>

std::string read_file(const std::string& filename) {
    std::ifstream ifs(filename);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

void write_file(const std::string& filename, const std::string& content) {
    std::ofstream ofs(filename);
    ofs << content;
    ofs.close();
}

std::string to_c_string(std::string str) {
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
        case '\'':   result += "\\\'"; break;
        case '\"':   result += "\\\""; break;
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
