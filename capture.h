#pragma once
#include "node.h"

class Alternation;

class Capture : public Node {
    std::shared_ptr<Alternation> expression;
public:
    Capture(const Alternation& expression, Node* parent);
    Capture(Parser2& p, Node* parent);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;
};
