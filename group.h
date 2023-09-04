#pragma once
#include "node.h"

class Alternation;

class Group : public Node<Group> {
    std::shared_ptr<Alternation> expression;
public:
    Group(const Alternation& expression);
    Group(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
