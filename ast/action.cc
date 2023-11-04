#include "action.h"
#include "utils.h"
#include "log.h"

Action::Action(const std::string& code, Node* parent) : Node("Action", parent), code(code) {}
Action::Action(Parser& p, Node* parent) : Node("Action", parent) {
    parse(p);
}

void Action::parse(Parser& p) {
    debug("Parsing Action");
    if (p.match_code()) {
        code = trim(p.last_match);
        valid = true;
    }
}

std::string Action::to_string() const {
    return "{ " + code + " }";
}

std::string Action::dump(std::string indent) const {
    return indent + "ACTION " + to_c_string(code);
}

bool Action::contains_var(const std::string& name) const {
    std::regex re(".*\\b" + name + "\\b.*");
    return std::regex_match(code, re);
}

bool operator==(const Action& a, const Action& b) {
    return a.code == b.code;
}

bool operator!=(const Action& a, const Action& b) {
    return !(a == b);
}
