#pragma once
#include "node.h"
#include "sequence.h"

class Alternation : public Node<Alternation> {
    std::vector<Sequence> sequences;
public:
    Alternation(const std::vector<Sequence>& sequences);
    Alternation(Parser2& p);
    Alternation();

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
