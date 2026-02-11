#pragma once
#include "ast/node.h"

class Predicate : public Node {
    std::string code;
    bool negative;
public:
    Predicate(Node* parent, std::string code, bool negative);
    Predicate(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual bool is_multiline() const override;

    bool contains_capture(int i) const;
    void renumber_capture(int from, int to);

    friend bool operator==(const Predicate& a, const Predicate& b);
};

bool operator==(const Predicate& a, const Predicate& b);
bool operator!=(const Predicate& a, const Predicate& b);
