#include "group.h"
#include "alternation.h"

Group::Group(const Alternation& expression, Node* parent) : Node("Group", parent), expression(new Alternation(expression)) {}
Group::Group(Parser2& p, Node* parent) : Node("Group", parent) {
    parse(p);
}

void Group::parse(Parser2& p) {
    //~ printf("parsing  Group\n");
    if (!p.match('(') && !p.match('<')) {
        return;
    }
    capture = p.last_match[0] == '<';
    expression.reset(new Alternation(p, this));
    if (!*expression) {
        return;
    }
    if (!p.match(capture ? '>' : ')')) {
        return;
    }
    valid = true;
}

std::string Group::to_string() const {
    char start = capture ? '<' : '(';
    char end = capture ? '>' : ')';
    return start + expression->to_string() + end;
}

std::string Group::dump(std::string indent) const {
    return indent + "GROUP" + (capture ? "capturing" : "") + "\n" + expression->dump(indent + "  ");
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
