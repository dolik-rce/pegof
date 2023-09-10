#include "rule.h"

Rule::Rule(const std::string& name, const Alternation& expression): Node("Rule"), name(name), expression(expression) {}
Rule::Rule(Parser2& p) : Node("Rule") { parse(p); }

void Rule::parse(Parser2& p) {
    //~ printf("parsing  Rule\n");
    parse_comments(p);
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
    return comments() + name + " <- " + expression.to_string() + "\n";
}

std::string Rule::dump(std::string indent) const {
    return indent + "RULE " + name + "\n" + expression.dump(indent + "  ");
}
