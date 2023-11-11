#pragma once
#include "ast/node.h"

class Alternation;

class Capture : public Node {
public:
    std::shared_ptr<Alternation> expression;
    int num;

    Capture(const Alternation& expression, Node* parent);
    Capture(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;

    bool has_single_term() const;
};

bool operator==(const Capture& a, const Capture& b);
bool operator!=(const Capture& a, const Capture& b);
