#pragma once
#include "ast/node.h"

class Expand : public Node {
public:
    int content;
    Expand(int content, Node* parent);
    Expand(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
};

bool operator==(const Expand& a, const Expand& b);
bool operator!=(const Expand& a, const Expand& b);
