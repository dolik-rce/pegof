#pragma once
#include "node.h"

class CharacterClass2 : public Node {
    std::string content;

    using Token = std::pair<int, int>;
    using Tokens = std::vector<Token>;

    bool dash;
    bool negation;
    Tokens tokens;

    void parseContent();
public:
    CharacterClass2(const std::string& content, Node* parent);
    CharacterClass2(Parser2& p, Node* parent);

    virtual void normalize();

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};
