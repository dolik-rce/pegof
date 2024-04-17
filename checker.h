#include "ast/grammar.h"
#include <string>

class Stats {
    int lines;
    int bytes;
    int rules;
    int terms;
    int duration;
    int memory;
public:
    Stats(int bytes, int lines, int rules, int terms, int duration, int memory)
        : bytes(bytes), lines(lines), rules(rules), terms(terms), duration(duration), memory(memory) {};
    std::string compare(const Stats& s) const;
};

class Checker {
    std::string tmp;
    std::string output;
    bool call_packcc(const std::string& input, const std::string& output, std::string& stderr) const;
    bool validate(const std::string& input) const;
    void benchmark(int& duration, int& memory) const;
public:
    Checker();
    ~Checker();

    bool packcc(const std::string& peg, const std::string& output) const;
    bool validate_string(const std::string& filename, const std::string& peg) const;
    bool validate_file(const std::string& filename) const;
    bool validate(const std::string& filename, const std::string& content) const;
    Stats stats(Grammar& g) const;
};
