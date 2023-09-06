#include "expand.h"

Expand::Expand(const std::string& content) : Node("Expand"), content(content) {}
Expand::Expand(Parser2& p) : Node("Expand") { parse(p); }

void Expand::parse(Parser2& p) {
    //~ printf("parsing Expand\n");
    p.skip_space();
    if (!p.match('$')) {
        return;
    } else if (p.match_number()) {
        content = p.last_match;
    } else {
        printf("ERROR: expected number!\n");
        exit(0);
    }
    valid = true;
}

std::string Expand::to_string() const {
    return content;
}

std::string Expand::dump(std::string indent) const {
    return indent + "EXPAND " + content;
}
