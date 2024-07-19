#pragma once
#include "ast/node.h"

class Directive: public Node {
    std::string name;
    std::string value;
    bool code;

public:
    Directive(const std::string& name, const std::string& value, bool code, Node* parent);
    Directive(Parser& p, Node* parent);

    bool is_import() const;
    std::string get_value() const;

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string indent = "") const override;
    virtual bool is_multiline() const override;
};
