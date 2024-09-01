#pragma once
#include "ast/node.h"
#include "ast/reference.h"
#include "ast/directive.h"
#include "ast/rule.h"
#include "ast/code.h"

using TopLevel = std::variant<std::monostate, Directive, Rule>;

class Grammar : public Node {
    std::vector<TopLevel> nodes;
    Code code;
    std::string input_file;
public:
    Grammar(
        const std::vector<TopLevel>& nodes,
        const Code& code,
        const std::string& input_file
    );
    Grammar(Parser& p, const std::string& input_file);
    Grammar(const std::string& p, const std::string& input_file);

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string indent = "") const override;
    virtual bool is_multiline() const override;

    virtual Node* operator[](int index) override;
    virtual long size() const override;

    void erase(Rule* rule);

    std::string dump_graph(const std::string& title) const;
};
