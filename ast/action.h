#pragma once
#include "ast/node.h"
#include "reference.h"

class Action : public Node {
public:
    std::string code;
    Action(const std::string& code, Node* parent);
    Action(Parser& p, Node* parent);

    virtual void parse(Parser& p);

    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;

    bool contains_var(const std::string& name) const;
};

bool operator==(const Action& a, const Action& b);
bool operator!=(const Action& a, const Action& b);
