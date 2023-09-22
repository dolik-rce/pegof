#pragma once
#include "ast/node.h"

class Alternation;

class Group : public Node {
public:
    bool capture;
    std::shared_ptr<Alternation> expression;

    Group(const Alternation& expression, Node* parent);
    Group(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;
};
