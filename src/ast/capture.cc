#include "ast/capture.h"
#include "ast/alternation.h"
#include "log.h"

Capture::Capture(const Alternation& expression, Node* parent) : Node("Capture", parent), expression(new Alternation(expression)), num(-1) {}
Capture::Capture(Parser& p, Node* parent) : Node("Capture", parent) {
    parse(p);
}

void Capture::parse(Parser& p) {
    debug("Parsing Capture");
    DebugIndent _;
    Parser::State s = p.save_point();
    p.skip_space();
    if (!p.match('<')) {
        s.rollback();
        return;
    }
    expression.reset(new Alternation(p, this));
    if (!*expression) {
        s.rollback();
        return;
    }
    p.skip_space();
    if (!p.match('>')) {
        s.rollback();
        return;
    }
    parse_post_comment(p);
    s.commit();
    valid = true;
}

std::string Capture::to_string(std::string indent) const {
    bool multiline = expression->size() > Config::get<int>("wrap-limit");
    std::string result;
    if (multiline) {
        result = "<\n" + expression->to_string(indent) + "\n" + indent.substr(0, indent.length() - 4) + ">";
    } else {
        result = "<" + expression->to_string(indent) + ">";
    }
    if (!post_comment.empty()) {
        result += " #" + post_comment;
    }
    return result;
}

std::string Capture::dump(std::string indent) const {
    return indent + "CAPTURE " + std::to_string(num) + dump_comments() + "\n" + expression->dump(indent + "  ");
}

Node* Capture::operator[](int index) {
    if (index == 0) {
        return (Node*)expression.get();
    } else {
        error("index out of bounds!");
    }
}

long Capture::size() const {
    return 1;
}

bool Capture::has_single_term() const {
    return expression->has_single_term();
}

Group Capture::convert_to_group() {
    return Group(*expression, nullptr);
}

bool operator==(const Capture& a, const Capture& b) {
    return *a.expression == *b.expression;
}

bool operator!=(const Capture& a, const Capture& b) {
    return !(a == b);
}
