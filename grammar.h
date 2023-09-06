#pragma once
#include "node.h"
#include "reference.h"
#include "directive.h"
#include "rule.h"

class Grammar : public Node {
public:
    std::vector<Directive> directives;
    std::vector<Rule> rules;
    std::string code;

    Grammar(
        const std::vector<Directive>& directives,
        const std::vector<Rule>& rules,
        const std::string& code
    );
    Grammar(Parser2& p);
    Grammar(const std::string& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;
};
