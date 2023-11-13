#pragma once
#include "ast/node.h"

class String : public Node {
    std::string content;
public:
    String(const std::string& content, Node* parent);
    String(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;

    const char* c_str() const;
    void append(const char* str);

    friend bool operator==(const String& a, const String& b);
};

bool operator==(const String& a, const String& b);
bool operator!=(const String& a, const String& b);
