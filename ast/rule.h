#pragma once
#include "ast/node.h"
#include "alternation.h"

class Rule : public Node {
public:
    std::string name;
    Alternation expression;

    Rule(const std::string& name, const Alternation& expression, Node* parent);
    Rule(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;

    bool is_terminal();
    bool contains_alternation();
    bool contains_expand();
    int count_terms();
    int count_cc_tokens();
    void update_captures();
};
