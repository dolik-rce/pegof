#pragma once
#include "node.h"

class Alternation;

class Group : public Node {
    std::shared_ptr<Alternation> expression;
public:
    Group(const Alternation& expression, Node* parent);
    Group(Parser2& p, Node* parent);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;
};
