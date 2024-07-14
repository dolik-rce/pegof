#include "utils.h"
#include "log.h"

#include <string.h>
#include <unistd.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <numeric>

#if defined(USE_EXPERIMENTAL_FILESYSTEM)
    #if defined(__cpp_lib_filesystem)
        #define USE_EXPERIMENTAL_FILESYSTEM 0
    #elif defined(__cpp_lib_experimental_filesystem)
        #define USE_EXPERIMENTAL_FILESYSTEM 1
    #elif !defined(__has_include)
        #define USE_EXPERIMENTAL_FILESYSTEM 1
    #elif __has_include(<filesystem>)
        #define USE_EXPERIMENTAL_FILESYSTEM 0
    #elif __has_include(<experimental/filesystem>)
        #define USE_EXPERIMENTAL_FILESYSTEM 1
    #else
        #error "Could not find <filesystem> or <experimental/filesystem> system header"
    #endif
#endif

#if USE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

TempDir::TempDir() {
    std::string tmp_dir_template = (fs::temp_directory_path() / "pegof_XXXXXX").native();
    char* filename = new char[tmp_dir_template.size() + 1];
    strcpy(filename, tmp_dir_template.c_str());
    fs::path tmp_dir = mkdtemp(filename);
    delete[] filename;
    fs::create_directory(tmp_dir);
    path = tmp_dir.native();
}

TempDir::~TempDir() {
    fs::remove_all(path);
}

std::string TempDir::get(const std::string& filename) {
    static TempDir instance;
    if (filename.empty()) {
        return instance.path;
    } else {
        return (fs::path(instance.path) / filename).native();
    }
}

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

std::string dirname(const std::string& path) {
    return fs::path(path).parent_path().native();
}

std::string find_file(const std::string& name, const std::vector<std::string> dirs) {
    for (const auto& dir: dirs) {
        std::string path = (dir.empty() ? "." : dir) + "/" + name;
        if (fs::exists(path)) {
            debug("Found '%s' in directory '%s'.", name.c_str(), dir.c_str());
            return path;
        }
        debug("File '%s' doesn't exist.", path.c_str());
    }
    return "";
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

std::string trim(const std::string& str, TrimType type, const char* whitespace) {
    size_t start = (type & TRIM_LEFT) ? str.find_first_not_of(whitespace) : 0;
    size_t end = (type & TRIM_RIGHT) ? str.find_last_not_of(whitespace) + 1 : str.size();
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

std::string join(const std::vector<std::string> &v, const std::string& delimiter) {
    if (v.empty()) {
        return "";
    }
    return std::accumulate(std::next(v.begin()), v.end(), v[0], [delimiter](const std::string& a, const std::string& b) {
        return a + delimiter + b;
    });
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
