#pragma once
#include "ast/node.h"

class Action;
class Rule;

class Reference : public Node {
    std::string name;
    std::string var;
public:
    Reference(const std::string& name, const std::string& var, Node* parent);
    Reference(Parser& p, Node* parent);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string indent = "") const override;
    virtual bool is_multiline() const override;

    std::string get_name() const;
    bool references(const Rule* rule) const;
    bool has_variable() const;
    void remove_variable();

    friend bool operator==(const Reference& a, const Reference& b);
    friend class Action;
};

bool operator==(const Reference& a, const Reference& b);
bool operator!=(const Reference& a, const Reference& b);
