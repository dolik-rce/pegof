#pragma once
#include "ast/node.h"
#include "ast/reference.h"
#include "ast/directive.h"
#include "ast/rule.h"
#include "ast/code.h"

class Grammar : public Node {
    std::vector<Directive> directives;
    std::vector<Rule> rules;
    Code code;
public:
    Grammar(
        const std::vector<Directive>& directives,
        const std::vector<Rule>& rules,
        const Code& code
    );
    Grammar(Parser& p);
    Grammar(const std::string& p);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;

    void erase(Rule* rule);
};
