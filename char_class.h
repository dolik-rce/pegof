#include <string>
#include <vector>

class CharacterClass {
    using Token = std::pair<int, int>;
    using Tokens = std::vector<Token>;


    static int get_char(const std::string& s, size_t& pos);
    static std::string to_char(int c);

    static Token get_range(const std::string& s, size_t& pos);
    static Tokens tokenize(const std::string& input);
    static Tokens join(const Tokens& tokens);

public:
    static std::string normalize(const std::string& str);
};
