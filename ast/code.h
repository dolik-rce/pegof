#pragma once
#include "ast/node.h"

class Code : public Node {
public:
    std::string content;

    Code(const std::string& content, Node* parent);
    Code(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;
};
