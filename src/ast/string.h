#pragma once
#include "ast/node.h"

class String: public Node {
    std::string content;

public:
    String(const std::string& content, Node* parent);
    String(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual bool is_multiline() const override;
    virtual size_t hash() const override;

    const char* c_str() const;
    std::string to_c_string() const;
    void append(const String& str);

    friend bool operator==(const String& a, const String& b);
};

bool operator==(const String& a, const String& b);
bool operator!=(const String& a, const String& b);
