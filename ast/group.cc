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
    parse_post_comment(p);
    s.commit();
    valid = true;
}

std::string Group::to_string(std::string indent) const {
    bool multiline = expression->size() > Config::get<int>("wrap-limit");
    std::string start = capture ? "<" : "(";
    std::string end = capture ? ">" : ")";
    if (multiline) {
        return start + "\n" + expression->to_string(indent) + "\n" + indent.substr(0, indent.length() - 4) + end;
    } else {
        return start + expression->to_string(indent) + end;
    }
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
