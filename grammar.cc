#include "grammar.h"
#include "utils.h"

Grammar::Grammar(
    const std::vector<Directive>& directives,
    const std::vector<Rule>& rules,
    const std::string& code
) : Node("Grammar"), directives(directives), rules(rules), code(code) {}

Grammar::Grammar(Parser2& p): Node("Grammar") {
    parse(p);
}

Grammar::Grammar(const std::string& s): Node("Grammar") {
    Parser2 p(s);
    parse(p);
}

void Grammar::parse(Parser2& p) {
    //~ printf("parsing  Grammar\n");
    parse_comments(p);
    while (true) {
        Directive d(p);
        if (d) {
            directives.push_back(d);
            continue;
        }
        Rule r(p);
        if (r) {
            rules.push_back(r);
            continue;
        }
        if (p.match("%%")) {
            p.match_re("[\\s\\S]*");
            code = p.last_re_match.str(0);
            break;
        }
        return;
    }
    valid = true;
}

std::string Grammar::to_string() const {
    std::string result = comments();
    for (const Directive& directive : directives) {
        result += directive.to_string();
    }
    for (const Rule& rule : rules) {
        result += rule.to_string();
    }
    result += "%%\n" + code;
    return result;
}

std::string Grammar::dump(std::string indent) const {
    std::string result = indent + "GRAMMAR\n";
    for (const Directive& directive : directives) {
        result += directive.dump(indent + "  ") + "\n";
    }
    for (const Rule& rule : rules) {
        result += rule.dump(indent + "  ") + "\n";
    }
    result += indent + "CODE: \"" + to_c_string(code) + "\"";
    return result;
}

Node* Grammar::operator[](int index) {
    if (index == 0) {
        return this;
    } else if (index <= directives.size()) {
        return &(directives[index - 1]);
    } else if (index <= directives.size() + rules.size()) {
        return &(rules[index - directives.size() - 1]);
    } else {
        printf("ERROR: index out of bounds!\n");
        exit(1);
    }
}

long Grammar::size() const {
    return 1 + directives.size() + rules.size();
}
