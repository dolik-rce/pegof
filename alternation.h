#pragma once
#include "node.h"
#include "sequence.h"

class Alternation : public Node {
public:
    std::vector<Sequence> sequences;
    Alternation(const std::vector<Sequence>& sequences);
    Alternation(Parser2& p);
    Alternation();

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;
};
