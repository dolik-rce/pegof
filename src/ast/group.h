#pragma once
#include "ast/node.h"

class Alternation;
class Sequence;
class Term;

class Group : public Node {
    std::shared_ptr<Alternation> expression;
public:
    Group(const Alternation& expression, Node* parent);
    Group(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual bool is_multiline() const override;

    virtual Node* operator[](int index) override;
    virtual long size() const override;

    bool has_single_sequence() const;
    bool has_single_term() const;
    const Sequence& get_first_sequence() const;
    const Term& get_first_term() const;
    const Alternation& convert_to_alternation() const;

    friend bool operator==(const Group& a, const Group& b);
};

bool operator==(const Group& a, const Group& b);
bool operator!=(const Group& a, const Group& b);
