#pragma once
#include <string>
#include <vector>

#define STR(x) (x).to_string().c_str()

class TempDir {
private:
    TempDir();
    ~TempDir();
    std::string path;
public:
    static std::string get(const std::string& filename);
};

std::string read_file(const std::string& filename);
void write_file(const std::string& filename, const std::string& content);
std::string dirname(const std::string& path);
std::string find_file(const std::string& name, const std::vector<std::string> dirs);

enum EscapeMode {
    ESCAPE_SINGLE_QUOTES = 1,
    ESCAPE_DOUBLE_QUOTES = 2,
    ESCAPE_ALL = ESCAPE_SINGLE_QUOTES | ESCAPE_DOUBLE_QUOTES
};

std::string to_hex(int number, int width);
std::string to_c_string(std::string str, EscapeMode mode = ESCAPE_ALL);

enum TrimType {
    TRIM_LEFT = 1,
    TRIM_RIGHT = 2,
    TRIM_BOTH = TRIM_LEFT | TRIM_RIGHT
};

std::string trim(const std::string& str, TrimType type = TRIM_BOTH, const char* whitespace = " \t\r\n");
std::vector<std::string> split(const std::string &s, const std::string& delimiter = "\\s*,\\s*");
std::string join(const std::vector<std::string> &v, const std::string& delimiter = "\n");
bool contains(std::vector<std::string> values, std::string x);
std::string replace(const std::string& input, const std::string& re, const std::string& replace);
std::string left_pad(const std::string& s, int width);
