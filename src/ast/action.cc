#include "ast/action.h"
#include "utils.h"
#include "log.h"
#include <regex>

Action::Action(const std::string& code, Node* parent) : Node("Action", parent), code(code) {}
Action::Action(Parser& p, Node* parent) : Node("Action", parent) {
    parse(p);
}

void Action::parse(Parser& p) {
    debug("Parsing Action");
    DebugIndent _;
    if (p.match_code()) {
        code = trim(p.last_match);
        valid = true;
    }
}

std::string Action::to_string(std::string indent) const {
    return "{ " + code + " }";
}

std::string Action::dump(std::string indent) const {
    return indent + "ACTION " + to_c_string(code);
}

bool Action::is_multiline() const {
    return code.find('\n') != std::string::npos;
}

bool Action::contains_reference(const Reference& ref) const {
    std::regex re("(.|\n)*\\b" + ref.var + "\\b(.|\n)*");
    return std::regex_match(code, re);
}

bool Action::contains_capture(int i) const {
    std::regex re("(.|\n)*\\$" + std::to_string(i) + "[se]?\\b(.|\n)*");
    return std::regex_match(code, re);
}

void Action::renumber_capture(int from, int to) {
    code = replace(code, "\\$" + std::to_string(from) + "\\b", "$$" + std::to_string(to));
    code = replace(code, "\\$" + std::to_string(from) + "s\\b", "$$" + std::to_string(to) + "s");
    code = replace(code, "\\$" + std::to_string(from) + "e\\b", "$$" + std::to_string(to) + "e");
}

bool operator==(const Action& a, const Action& b) {
    return a.code == b.code;
}

bool operator!=(const Action& a, const Action& b) {
    return !(a == b);
}
