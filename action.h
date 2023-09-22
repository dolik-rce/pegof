#pragma once
#include "node.h"
#include "reference.h"

class Action : public Node {
    std::string code;
public:
    Action(const std::string& code, Node* parent);
    Action(Parser& p, Node* parent);

    virtual void parse(Parser& p);

    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};
