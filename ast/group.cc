#include "group.h"
#include "alternation.h"
#include "log.h"

Group::Group(const Alternation& expression, Node* parent) : Node("Group", parent), expression(new Alternation(expression)) {}
Group::Group(Parser& p, Node* parent) : Node("Group", parent) {
    parse(p);
}

void Group::parse(Parser& p) {
    debug("Parsing Group");
    Parser::State s = p.save_point();
    parse_comments(p);
    if (!p.match('(') && !p.match('<')) {
        s.rollback();
        return;
    }
    capture = p.last_match[0] == '<';
    expression.reset(new Alternation(p, this));
    if (!*expression) {
        s.rollback();
        return;
    }
    p.skip_space();
    if (!p.match(capture ? '>' : ')')) {
        s.rollback();
        return;
    }
    parse_comments(p, true);
    s.commit();
    valid = true;
}

std::string Group::to_string() const {
    char start = capture ? '<' : '(';
    char end = capture ? '>' : ')';
    return start + expression->to_string() + end;
}

std::string Group::dump(std::string indent) const {
    return indent + "GROUP" + (capture ? "capturing" : "") + dump_comments() + "\n" + expression->dump(indent + "  ");
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
