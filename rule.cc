#include "rule.h"

Rule::Rule(const std::string& name, const Alternation& expression): name(name), expression(expression) {}
Rule::Rule(Parser2& p) { parse(p); }

void Rule::parse(Parser2& p) {
    printf("parsing  Rule\n");
    if (!p.match_re("(\\S+)\\s*<-")) {
        return;
    }
    name = p.last_re_match.str(1);
    expression = Alternation(p);
    if (!expression) {
        return;
    }
    valid = true;
}

std::string Rule::to_string() const {
    return "RULE " + name + " <- " + expression.to_string() + "\n";
}
