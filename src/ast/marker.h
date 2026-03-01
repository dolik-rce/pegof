#pragma once
#include "ast/node.h"

class Marker: public Node {
    std::string name;

public:
    Marker(const std::string& name, Node* parent);
    Marker(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual bool is_multiline() const override;
    virtual size_t hash() const override;

    friend bool operator==(const Marker& a, const Marker& b);
};

bool operator==(const Marker& a, const Marker& b);
bool operator!=(const Marker& a, const Marker& b);
