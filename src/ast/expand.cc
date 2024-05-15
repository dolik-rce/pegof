#include "ast/expand.h"
#include "log.h"

Expand::Expand(int content, Node* parent) : Node("Expand", parent), content(content) {}
Expand::Expand(Parser& p, Node* parent) : Node("Expand", parent) {
    parse(p);
}

void Expand::parse(Parser& p) {
    debug("Parsing Expand");
    DebugIndent _;
    Parser::State s = p.save_point();
    p.skip_space();
    if (!p.match('$')) {
        s.rollback();
        return;
    } else if (p.match_number()) {
        content = std::stoi(p.last_match);
    } else {
        error("expected number!");
    }
    s.commit();
    valid = true;
}

std::string Expand::to_string(std::string indent) const {
    return "$" + std::to_string(content);
}

std::string Expand::dump(std::string indent) const {
    return indent + "EXPAND " + std::to_string(content);
}

bool Expand::is_multiline() const {
    return false;
}

void Expand::shift(int n) {
    content += n;
}

bool operator==(const Expand& a, const Expand& b) {
    return a.content == b.content;
}

bool operator!=(const Expand& a, const Expand& b) {
    return !(a == b);
}

bool operator==(const Expand& a, const int b) {
    return a.content == b;
}

bool operator<=(const Expand& a, const int b) {
    return a.content <= b;
}
