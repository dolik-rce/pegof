#include <string>

std::string read_file(const std::string& filename);
void write_file(const std::string& filename, const std::string& content);

std::string to_c_string(std::string str);

enum TrimType {
    TRIM_LEFT = 1,
    TRIM_RIGHT = 2,
    TRIM_BOTH = TRIM_LEFT | TRIM_RIGHT
};

std::string trim(const std::string& str, TrimType type = TRIM_BOTH);
