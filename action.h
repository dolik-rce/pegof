#pragma once
#include "node.h"
#include "reference.h"

class Action : public Node {
    std::string code;
public:
    Action(const std::string& code);
    Action(Parser2& p);

    virtual void parse(Parser2& p);

    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};
