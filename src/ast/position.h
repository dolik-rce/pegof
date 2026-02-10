#pragma once
#include "ast/node.h"

class Position : public Node {
public:
    Position(Node* parent);
    Position(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual bool is_multiline() const override;

    friend bool operator==(const Position& a, const Position& b);
};

bool operator==(const Position& a, const Position& b);
bool operator!=(const Position& a, const Position& b);
