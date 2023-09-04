#pragma once
#include "node.h"

class String : public Node<String> {
    std::string content;
public:
    String(const std::string& content);
    String(Parser2& p);
    String();

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
