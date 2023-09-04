#include "action.h"

Action::Action(const std::string& code) : code(code) {}
Action::Action(Parser2& p) { parse(p); }

void Action::parse(Parser2& p) {
    printf("parsing Action\n");
    p.skip_space();
    if (p.match_code()) {
        code = p.last_match;
        valid = true;
    }
}

std::string Action::to_string() const {
    return "ACTION " + code;
}
