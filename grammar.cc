#include "grammar.h"

Grammar::Grammar(
    const std::vector<Directive>& directives,
    const std::vector<Rule>& rules,
    const std::string& code
) : directives(directives), rules(rules), code(code) {}

Grammar::Grammar(Parser2& p) {
    parse(p);
}

void Grammar::parse(Parser2& p) {
    printf("parsing  Grammar\n");
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
    std::string result;
    for (const Directive& directive : directives) {
        result += directive.to_string();
    }
    for (const Rule& rule : rules) {
        result += rule.to_string();
    }
    result += "%%\n" + code;
    return result;
}
