#pragma once
#include "ast/node.h"
#include "ast/string.h"

class CharacterClass : public Node {
    std::string content;

    using Token = std::pair<int, int>;
    using Tokens = std::vector<Token>;

    bool dash;
    bool negation;
    Tokens tokens;

    void parse_content(Parser& p);
    void tokenize();
    void update_content();
public:
    CharacterClass(const std::string& content, Node* parent);
    CharacterClass(Parser& p, Node* parent);

    bool normalize();
    void flip_negation();

    bool any_char() const;
    int token_count() const;
    bool is_single_char() const;
    bool is_negative() const;

    String convert_to_string() const;
    void merge(const CharacterClass& cc);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual bool is_multiline() const override;

    friend bool operator==(const CharacterClass& a, const CharacterClass& b);
};

bool operator==(const CharacterClass& a, const CharacterClass& b);
bool operator!=(const CharacterClass& a, const CharacterClass& b);
