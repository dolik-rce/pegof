#include "ast/predicate.h"
#include "utils.h"
#include "config.h"
#include "log.h"
#include <cstdint>

Predicate::Predicate(Node* parent, std::string code, bool negative) : Node("Predicate", parent), code(code), negative(negative) {}
Predicate::Predicate(Parser& p, Node* parent) : Node("Predicate", parent) {
    parse(p);
}

void Predicate::parse(Parser& p) {
    debug("Parsing Predicate");
    DebugIndent _;
    if (!p.peek_re("[!&]\\{")) {
        return;
    }
    if (p.match('!')) {
        negative = true;
    } else {
        p.match('&');
        negative = false;
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

std::string Predicate::to_string(std::string indent) const {
    if (is_multiline()) {
        return (negative ? "!{\n    " : "&{\n    ") + indent + replace(code, "\n", "\n    " + indent) + "\n" + indent + "}";
    } else {
        return (negative ? "!{ " : "&{ ") + code + " }";
    }
}

std::string Predicate::dump(std::string indent) const {
    return indent + "PREDICATE " + (negative ? "! " : "") + to_c_string(code);
}

bool Predicate::is_multiline() const {
    return code.find('\n') != std::string::npos || code.substr(0, 1) == "#";
}

bool Predicate::contains_capture(int i) const {
    std::regex re("(.|\n)*\\$" + std::to_string(i) + "[se]?\\b(.|\n)*");
    return std::regex_match(code, re);
}

void Predicate::renumber_capture(int from, int to) {
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

bool operator==(const Predicate& a, const Predicate& b) {
    return a.code == b.code && a.negative == b.negative;
}
bool operator!=(const Predicate& a, const Predicate& b) {
    return !(a == b);
}
