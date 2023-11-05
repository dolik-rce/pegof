#pragma once
#include "ast/node.h"

class CharacterClass : public Node {
public:
    std::string content;

    using Token = std::pair<int, int>;
    using Tokens = std::vector<Token>;

    bool dash;
    bool negation;
    Tokens tokens;

private:
    void parse_content(Parser& p);
    void update_content();
public:
    CharacterClass(const std::string& content, Node* parent);
    CharacterClass(Parser& p, Node* parent);

    virtual void normalize();

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
};

bool operator==(const CharacterClass& a, const CharacterClass& b);
bool operator!=(const CharacterClass& a, const CharacterClass& b);
