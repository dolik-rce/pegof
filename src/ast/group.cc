#include "ast/group.h"
#include "ast/alternation.h"
#include "log.h"

Group::Group(const Alternation& expression, Node* parent) : Node("Group", parent), expression(new Alternation(expression)) {}
Group::Group(Parser& p, Node* parent) : Node("Group", parent) {
    parse(p);
}

void Group::parse(Parser& p) {
    debug("Parsing Group");
    DebugIndent _;
    Parser::State s = p.save_point();
    p.skip_space();
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
    std::string result;
    if (is_multiline()) {
        result = "(\n" + expression->to_string(indent + "    ") + "\n" + indent + ")";
    } else {
        result = "(" + expression->to_string(indent) + ")";
    }
    if (!post_comment.empty()) {
        result += " #" + post_comment;
    }
    return result;
}

std::string Group::dump(std::string indent) const {
    return indent + "GROUP" + dump_comments() + "\n" + expression->dump(indent + "  ");
}

bool Group::is_multiline() const {
    return expression->is_multiline();
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

bool Group::has_single_sequence() const {
    return expression->size() == 1;
}

bool Group::has_single_term() const {
    return expression->has_single_term();
}

const Sequence& Group::get_first_sequence() const {
    return expression->get_first_sequence();
}

const Term& Group::get_first_term() const {
    return get_first_sequence().get_first_term();
}

const Alternation& Group::convert_to_alternation() const {
    return *expression;
}

bool operator==(const Group& a, const Group& b) {
    return *a.expression == *b.expression;
}

bool operator!=(const Group& a, const Group& b) {
    return !(a == b);
}
