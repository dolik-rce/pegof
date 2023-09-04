#include "group.h"
#include "alternation.h"

Group::Group(const Alternation& expression) : expression(new Alternation(expression)) {}
Group::Group(Parser2& p) { parse(p); }

void Group::parse(Parser2& p) {
    printf("parsing  Group\n");
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
    return "GROUP (" + expression->to_string() + ")";
}
