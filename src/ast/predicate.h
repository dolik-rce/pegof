#pragma once
#include "ast/action.h"

class Predicate: public Action {
    bool negative;

public:
    Predicate(Node* parent, std::string code, bool negative);
    Predicate(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual size_t hash() const override;

    friend bool operator==(const Predicate& a, const Predicate& b);
};

bool operator==(const Predicate& a, const Predicate& b);
bool operator!=(const Predicate& a, const Predicate& b);
