#pragma once
#include "ast/node.h"
#include "ast/reference.h"

class Action : public Node {
    std::string code;
public:
    Action(const std::string& code, Node* parent);
    Action(const Action& action, Node* parent);
    Action(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;

    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;
    virtual bool is_multiline() const override;

    bool is_empty() const;
    bool contains_reference(const Reference& ref) const;
    bool contains_capture(int i) const;
    bool contains_any_capture() const;
    void renumber_capture(int from, int to);

    friend bool operator==(const Action& a, const Action& b);
};

bool operator==(const Action& a, const Action& b);
bool operator!=(const Action& a, const Action& b);
