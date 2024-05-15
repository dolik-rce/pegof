#pragma once
#include "ast/node.h"
#include "ast/sequence.h"

class Alternation : public Node {
    std::vector<Sequence> sequences;
public:
    Alternation(const std::vector<Sequence>& sequences, Node* parent);
    Alternation(Parser& p, Node* parent);
    Alternation(Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string indent = "") const override;
    virtual bool is_multiline() const override;

    Sequence& get(int index);
    virtual Node* operator[](int index);
    virtual long size() const;

    bool has_single_term() const;
    const Sequence& get_first_sequence() const;

    void insert(int index, const Alternation& a);
    void erase(int index);
    void erase(Sequence* s);

    friend bool operator==(const Alternation& a, const Alternation& b);
};

bool operator==(const Alternation& a, const Alternation& b);
bool operator!=(const Alternation& a, const Alternation& b);
