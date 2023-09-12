#include "action.h"
#include "utils.h"

Action::Action(const std::string& code) : Node("Action"), code(code) {}
Action::Action(Parser2& p) : Node("Action") { parse(p); }

void Action::parse(Parser2& p) {
    //~ printf("parsing Action\n");
    p.skip_space();
    if (p.match_code()) {
        code = p.last_match;
        valid = true;
    }
}

std::string Action::to_string() const {
    return "{ " + code + " }";
}

std::string Action::dump(std::string indent) const {
    return indent + "ACTION " + to_c_string(code);
}
