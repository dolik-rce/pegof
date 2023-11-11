#include "group.h"
#include "alternation.h"
#include "log.h"

Group::Group(const Alternation& expression, Node* parent) : Node("Group", parent), expression(new Alternation(expression)) {}
Group::Group(Parser& p, Node* parent) : Node("Group", parent) {
    parse(p);
}

void Group::parse(Parser& p) {
    debug("Parsing Group");
    DebugIndent _;
    Parser::State s = p.save_point();
    parse_comments(p);
    if (!p.match('(')) {
        s.rollback();
        return;
    }
    expression.reset(new Alternation(p, this));
    if (!*expression) {
        s.rollback();
        return;
    }
    p.skip_space();
    if (!p.match(')')) {
        s.rollback();
        return;
    }
    parse_post_comment(p);
    s.commit();
    valid = true;
}

std::string Group::to_string(std::string indent) const {
    bool multiline = expression->size() > Config::get<int>("wrap-limit");
    if (multiline) {
        return "(\n" + expression->to_string(indent) + "\n" + indent.substr(0, indent.length() - 4) + ")";
    } else {
        return "(" + expression->to_string(indent) + ")";
    }
}

std::string Group::dump(std::string indent) const {
    return indent + "GROUP" + dump_comments() + "\n" + expression->dump(indent + "  ");
}

Node* Group::operator[](int index) {
    if (index == 0) {
        return (Node*)expression.get();
    } else {
        error("index out of bounds!");
    }
}

long Group::size() const {
    return 1;
}

bool Group::has_single_term() const {
    return expression->has_single_term();
}

bool operator==(const Group& a, const Group& b) {
    return *a.expression == *b.expression;
}

bool operator!=(const Group& a, const Group& b) {
    return !(a == b);
}
