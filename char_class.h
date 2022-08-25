#include <string>
#include <vector>

class CharacterClass {
    using Token = std::pair<int, int>;
    using Tokens = std::vector<Token>;

    Token get_range(const std::string& s, size_t& pos) const;
    int get_char(const std::string& s, size_t& pos) const;

    std::string to_char(int c) const;
    std::string to_unicode(const int c) const;

    bool dash;
    bool negation;
    Tokens tokens;
public:
    CharacterClass& normalize();
    CharacterClass& toggle_negation();
    bool negative() const;
    std::string to_string() const;
    int size() const;

    CharacterClass(const std::string str);
};
