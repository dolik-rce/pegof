#pragma once
#include "node.h"

class String : public Node {
public:
    std::string content;
    String(const std::string& content, Node* parent);
    String(Parser2& p, Node* parent);
    //~ String();

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};
