#pragma once
#include "ast/node.h"
#include "ast/term.h"

class Sequence: public Node {
    std::vector<Term> terms;

public:
    Sequence(const std::vector<Term>& terms, Node* parent);
    Sequence(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string indent = "") const override;
    virtual bool is_multiline() const override;

    virtual Node* operator[](int index) override;
    virtual long size() const override;

    Term& get(int index);

    bool has_single_term() const;
    const Term& get_first_term() const;
    Term& get_first_term();

    void insert(int index, const Sequence& s);
    void erase(Term* rule);
    void erase(int index);

    friend bool operator==(const Sequence& a, const Sequence& b);
};

bool operator==(const Sequence& a, const Sequence& b);
bool operator!=(const Sequence& a, const Sequence& b);
