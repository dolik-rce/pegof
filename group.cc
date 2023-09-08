#include "group.h"
#include "alternation.h"

Group::Group(const Alternation& expression) : Node("Group"), expression(new Alternation(expression)) {}
Group::Group(Parser2& p) : Node("Group") { parse(p); }

void Group::parse(Parser2& p) {
    //~ printf("parsing  Group\n");
    if (!p.match("(")) {
        return;
    }
    expression.reset(new Alternation(p));
    if (!*expression) {
        return;
    }
    if (!p.match(")")) {
        return;
    }
    valid = true;
}

std::string Group::to_string() const {
    return "( " + expression->to_string() + " )";
}

std::string Group::dump(std::string indent) const {
    return indent + "GROUP\n" + expression->dump(indent + "  ");
}

Node* Group::operator[](int index) {
    if (index == 0) {
        return this;
    } else if (index == 1) {
        return (Node*)expression.get();
    } else {
        printf("ERROR: index out of bounds!\n");
        exit(1);
    }
}

long Group::size() const {
    return 2;
}
