#include "rule.h"
#include "config.h"

Rule::Rule(const std::string& name, const Alternation& expression, Node* parent) : Node("Rule", parent), name(name), expression(expression) {}
Rule::Rule(Parser& p, Node* parent) : Node("Rule", parent), expression(this) {
    parse(p);
}

void Rule::parse(Parser& p) {
    //~ printf("parsing  Rule\n");
    Parser::State s = p.save_point();
    parse_comments(p);
    if (!p.match_re("(\\S+)\\s*<-")) {
        //~ printf("doesn't look like Rule\n");
        s.rollback();
        return;
    }
    name = p.last_re_match.str(1);
    expression = Alternation(p, this);
    if (!expression) {
        //~ printf("invalid expresion for Rule %s\n", name.c_str());
        s.rollback();
        return;
    }
    valid = true;
    //~ printf("parsed Rule %s\n", name.c_str());
}

std::string Rule::to_string() const {
    std::string result = format_comments() + (comments.empty() ? "" : "\n") + name + " <-";
    if (expression.sequences.size() > Config::get<int>("wrap-limit")) {
        result += "\n    ";
    } else {
        result += " ";
    }
    result += expression.to_string();
    return result;
}

std::string Rule::dump(std::string indent) const {
    std::string comments_info = " (" + std::to_string(comments.size()) + " comments)";
    return indent + "RULE " + name + comments_info + "\n" + expression.dump(indent + "  ");
}

Node* Rule::operator[](int index) {
    if (index == 0) {
        return &expression;
    } else {
        printf("ERROR: index out of bounds!\n");
        exit(1);
    }
}
long Rule::size() const {
    return 1;
}

bool Rule::is_terminal() {
    if (expression.sequences.size() != 1) return false;
    if (expression.sequences[0].terms.size() != 1) return false;
    return true;
}
