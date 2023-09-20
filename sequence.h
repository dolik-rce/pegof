#pragma once
#include "node.h"
#include "term.h"

class Sequence : public Node {
public:
    std::vector<Term> terms;

    Sequence(const std::vector<Term>& terms);
    Sequence(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;
};
