#pragma once
#include "ast/node.h"

class String : public Node {
public:
    std::string content;
    String(const std::string& content, Node* parent);
    String(Parser& p, Node* parent);
    //~ String();

    virtual void parse(Parser& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string = "") const override;
};

bool operator==(const String& a, const String& b);
bool operator!=(const String& a, const String& b);
