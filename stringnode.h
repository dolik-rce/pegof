#pragma once
#include "node.h"

class String : public Node {
    std::string content;
public:
    String(const std::string& content);
    String(Parser2& p);
    String();

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};
