#pragma once
#include "node.h"

class CharacterClass2 : public Node<CharacterClass2> {
    std::string content;
public:
    CharacterClass2(const std::string& content);
    CharacterClass2(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
