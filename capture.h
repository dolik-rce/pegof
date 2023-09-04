#pragma once
#include "node.h"

class Alternation;

class Capture : public Node<Capture> {
    std::shared_ptr<Alternation> expression;
public:
    Capture(const Alternation& expression);
    Capture(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
