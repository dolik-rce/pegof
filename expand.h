#pragma once
#include "node.h"

class Expand : public Node {
    std::string content; // TODO: shoud be number
public:
    Expand(const std::string& content);
    Expand(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};
