#pragma once
#include "node.h"

class Expand : public Node<Expand> {
    std::string content; // TODO: shoud be number
public:
    Expand(const std::string& content);
    Expand(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
