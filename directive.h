#pragma once
#include "node.h"

class Directive : public Node {
    std::string name;
    std::string value;
    bool code;
public:
    Directive(const std::string& name, const std::string& value, bool code, Node* parent);
    Directive(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;
};
