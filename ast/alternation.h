#pragma once
#include "ast/node.h"
#include "sequence.h"

class Alternation : public Node {
public:
    std::vector<Sequence> sequences;
    Alternation(const std::vector<Sequence>& sequences, Node* parent);
    Alternation(Parser& p, Node* parent);
    Alternation(Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;
};
