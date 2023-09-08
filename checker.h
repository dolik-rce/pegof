#include <string>

class Checker {
    std::string tmp;
    std::string output;
    bool call_packcc(const std::string& input, std::string& stderr) const;
    bool validate(const std::string& input) const;
public:
    Checker();
    ~Checker();

    bool validate_string(const std::string& peg) const;
    bool validate_file(const std::string& filename) const;
};
