#pragma once
#include "node.h"
#include "alternation.h"

class Rule : public Node<Rule> {
    std::string name;
    // replace this with actual nodes
    Alternation expression;
public:
    Rule(const std::string& name, const Alternation& expression);
    Rule(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
