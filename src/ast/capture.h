#pragma once
#include "ast/node.h"

class Alternation;
class Rule;
class Group;

class Capture : public Node {
    std::shared_ptr<Alternation> expression;
    int num;
public:
    Capture(const Alternation& expression, Node* parent);
    Capture(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual bool is_multiline() const override;

    virtual Node* operator[](int index) override;
    virtual long size() const override;

    bool has_single_term() const;

    Group convert_to_group();

    friend bool operator==(const Capture& a, const Capture& b);
    friend class Rule;
};

bool operator==(const Capture& a, const Capture& b);
bool operator!=(const Capture& a, const Capture& b);
