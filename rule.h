#pragma once
#include "node.h"
#include "alternation.h"

class Rule : public Node {
public:
    std::string name;
    // replace this with actual nodes
    Alternation expression;

    Rule(const std::string& name, const Alternation& expression, Node* parent);
    Rule(Parser2& p, Node* parent);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index) {
        if (index == 0) {
            return this;
        } else if (index == 1) {
            return &expression;
        } else {
            printf("ERROR: index out of bounds!\n");
            exit(1);
        }
    }
    virtual long size() const {
        return 2;
    }
};
