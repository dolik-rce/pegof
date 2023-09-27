#pragma once
#include "ast/node.h"

class Expand : public Node {
public:
    std::string content; // TODO: shoud be number
    Expand(const std::string& content, Node* parent);
    Expand(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};

bool operator==(const Expand& a, const Expand& b);
bool operator!=(const Expand& a, const Expand& b);
