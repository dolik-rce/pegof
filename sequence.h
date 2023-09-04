#pragma once
#include "node.h"
#include "term.h"

class Sequence : public Node<Sequence> {
    std::vector<Term> terms;
public:
    Sequence(const std::vector<Term>& terms);
    Sequence(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
