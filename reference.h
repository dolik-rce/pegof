#pragma once
#include "node.h"

class Reference : public Node {
    std::string name;
    std::string var;
    Reference(const std::string& name, const std::string& var);
public:
    Reference(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;
};
