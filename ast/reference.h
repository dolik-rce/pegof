#pragma once
#include "ast/node.h"

class Reference : public Node {
public:
    std::string name;
    std::string var;
    Reference(const std::string& name, const std::string& var, Node* parent);
    Reference(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;
};

bool operator==(const Reference& a, const Reference& b);
bool operator!=(const Reference& a, const Reference& b);
