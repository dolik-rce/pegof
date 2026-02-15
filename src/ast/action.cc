#include "ast/action.h"

#include "log.h"
#include "utils.h"

#include <cstdint>
#include <regex>

Action::Action(Node* parent): Node("Action", parent) {}
Action::Action(const std::string& code, Node* parent): Node("Action", parent), code(code) {}
Action::Action(const Action& action, Node* parent): Action(action.code, parent) {}
Action::Action(Parser& p, Node* parent): Node("Action", parent) {
    parse(p);
}

void Action::parse(Parser& p) {
    if (is<Action>()) {
        // Skip this when parsing Predicate to avoid confusion
        debug("Parsing Action");
        DebugIndent _;
    }
    if (p.match_code()) {
        code = trim(p.last_match);
        if (is_multiline()) {
            size_t min_indent = SIZE_MAX;
            std::vector<std::string> lines = split(trim(p.last_match, TRIM_BOTH, " \t"), "\n");
            if (lines[0].empty()) {
                lines.erase(lines.begin());
            }
            for (const std::string& line: lines) {
                min_indent = std::min(min_indent, line.size() - trim(line, TRIM_LEFT, " \t").size());
            }
            for (std::string& line: lines) {
                line = line.substr(min_indent);
            }
            code = join(lines);
        }
        valid = true;
    }
}

std::string Action::to_string(std::string indent) const {
    if (is_multiline()) {
        return "{\n    " + indent + replace(code, "\n", "\n    " + indent) + "\n" + indent + "}";
    } else {
        return "{ " + code + " }";
    }
}

std::string Action::dump(std::string indent) const {
    return indent + "ACTION " + to_c_string(code);
}

bool Action::is_empty() const {
    return std::regex_match(code, std::regex("\\s*"));
}

bool Action::is_multiline() const {
    return code.find('\n') != std::string::npos || code.substr(0, 1) == "#";
}

bool Action::contains_reference(const Reference& ref) const {
    std::regex re("(.|\n)*\\b" + ref.var + "\\b(.|\n)*");
    return std::regex_match(code, re);
}

bool Action::contains_capture(int i) const {
    std::regex re("(.|\n)*\\$" + std::to_string(i) + "[se]?\\b(.|\n)*");
    return std::regex_match(code, re);
}

bool Action::contains_any_capture() const {
    std::regex re("(.|\n)*\\$[0-9]+[se]?\\b(.|\n)*");
    return std::regex_match(code, re);
}

void Action::renumber_capture(int from, int to) {
    if (from == to) {
        return;
    }
    std::string f = "\\$" + std::to_string(from) + "([se]*)\\b";
    std::string t = "$" + std::to_string(to);
    std::string result;

    Parser p(code);
    while (!p.is_eof()) {
        if (p.match_re(f, false)) {
            result += t + p.last_re_match.str(1);
        } else if (p.match_string(false)) {
            result += "\"" + to_c_string(p.last_match) + "\"";
        } else if (p.match_block_comment(false) || p.match_line_comment(false)) {
            result += p.last_match;
        } else if (p.match_any_char()) {
            result += p.last_match;
        }
    }
    code = result;
}

bool operator==(const Action& a, const Action& b) {
    return a.code == b.code;
}

bool operator!=(const Action& a, const Action& b) {
    return !(a == b);
}
