#pragma once
#include "node.h"
#include "reference.h"
#include "directive.h"
#include "rule.h"

class Grammar : public Node<Reference> {
    std::vector<Directive> directives;
    std::vector<Rule> rules;
    std::string code;

public:
    Grammar(
        const std::vector<Directive>& directives,
        const std::vector<Rule>& rules,
        const std::string& code
    );
    Grammar(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
};
