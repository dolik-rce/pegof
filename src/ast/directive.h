#pragma once
#include "ast/node.h"

class Directive: public Node {
    std::string name;
    std::string value;
    std::string version;
    enum Type { CODE, IMPORT, STRING, MARKER, VERSION };
    Type type;

public:
    Directive(const std::string& name, const std::string& value, const std::string& version, Type type, Node* parent);
    Directive(Parser& p, Node* parent);

    bool is_import() const;
    bool is_version() const;
    std::string get_value() const;

    virtual void parse(Parser& p) override;
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string indent = "") const override;
    virtual bool is_multiline() const override;
    virtual size_t hash() const override;
};
