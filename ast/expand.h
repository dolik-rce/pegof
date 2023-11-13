#pragma once
#include "ast/node.h"

class Expand : public Node {
    int content;
public:
    Expand(int content, Node* parent);
    Expand(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;

    void shift(int n);

    friend bool operator==(const Expand& a, const Expand& b);
    friend bool operator==(const Expand& a, const int b);
    friend bool operator<=(const Expand& a, const int b);
};

bool operator==(const Expand& a, const Expand& b);
bool operator!=(const Expand& a, const Expand& b);

bool operator==(const Expand& a, const int b);
bool operator<=(const Expand& a, const int b);
