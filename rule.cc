#include "rule.h"
#include "config.h"

Rule::Rule(const std::string& name, const Alternation& expression, Node* parent): Node("Rule", parent), name(name), expression(expression) {}
Rule::Rule(Parser2& p, Node* parent) : Node("Rule", parent) { parse(p); }

void Rule::parse(Parser2& p) {
    //~ printf("parsing  Rule\n");
    parse_comments(p);
    if (!p.match_re("(\\S+)\\s*<-")) {
        return;
    }
    name = p.last_re_match.str(1);
    expression = Alternation(p, this);
    if (!expression) {
        return;
    }
    valid = true;
}

std::string Rule::to_string() const {
    std::string result = comments() + name + " <- ";
    if (expression.sequences.size() > Config::get<int>("wrap-limit")) {
         result += "\n    ";
    }
    result += expression.to_string() + "\n\n";
    return result;
}

std::string Rule::dump(std::string indent) const {
    return indent + "RULE " + name + "\n" + expression.dump(indent + "  ");
}
