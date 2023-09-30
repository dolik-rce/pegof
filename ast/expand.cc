#include "expand.h"
#include "log.h"

Expand::Expand(int content, Node* parent) : Node("Expand", parent), content(content) {}
Expand::Expand(Parser& p, Node* parent) : Node("Expand", parent) {
    parse(p);
}

void Expand::parse(Parser& p) {
    debug("Parsing Expand");
    p.skip_space();
    if (!p.match('$')) {
        return;
    } else if (p.match_number()) {
        content = std::stoi(p.last_match);
    } else {
        error("expected number!");
    }
    valid = true;
}

std::string Expand::to_string() const {
    return "$" + std::to_string(content);
}

std::string Expand::dump(std::string indent) const {
    return indent + "EXPAND " + std::to_string(content);
}

bool operator==(const Expand& a, const Expand& b) {
    return a.content == b.content;
}

bool operator!=(const Expand& a, const Expand& b) {
    return !(a == b);
}
