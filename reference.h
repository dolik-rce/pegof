#pragma once
#include "node.h"

class Reference : public Node<Reference> {
    std::string name;
    std::string var;
    Reference(const std::string& name, const std::string& var);
public:
    Reference(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
