#pragma once
#include "node.h"

class Expand : public Node {
    std::string content; // TODO: shoud be number
public:
    Expand(const std::string& content, Node* parent);
    Expand(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};
