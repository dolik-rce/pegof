#include "ast/rule.h"
#include "config.h"
#include "log.h"

Rule::Rule(const std::string& name, const Alternation& expression, Node* parent) : Node("Rule", parent), name(name), expression(expression) {}
Rule::Rule(Parser& p, Node* parent) : Node("Rule", parent), expression(this) {
    parse(p);
}

void Rule::parse(Parser& p) {
    debug("Parsing Rule");
    DebugIndent _;
    Parser::State s = p.save_point();
    parse_comments(p);
    if (!p.match_re("(\\S+)\\s*<-")) {
        s.rollback();
        return;
    }
    name = p.last_re_match.str(1);
    expression = Alternation(p, this);
    if (!expression) {
        s.rollback();
        return;
    }
    update_captures();
    valid = true;
}

std::string Rule::to_string(std::string indent) const {
    std::string result = format_comments() + (comments.empty() ? "" : "\n") + name + " <-";
    if (is_multiline()) {
        result += "\n" + expression.to_string("    ");
    } else {
        result += " " + expression.to_string();
    }
    return result;
}

std::string Rule::dump(std::string indent) const {
    return indent + "RULE " + name + dump_comments() + "\n" + expression.dump(indent + "  ");
}

bool Rule::is_multiline() const {
    if (expression.to_string().find('\n') == std::string::npos) {
        return false;
    }
    return expression.is_multiline();
}

Node* Rule::operator[](int index) {
    if (index == 0) {
        return &expression;
    } else {
        error("index out of bounds!");
    }
}

long Rule::size() const {
    return 1;
}

const char* Rule::c_str() const {
    return name.c_str();
}

bool Rule::is_terminal() const {
    if (expression.size() != 1) return false;
    if (expression.get_first_sequence().size() != 1) return false;
    return true;
}

Group Rule::convert_to_group() const {
    return Group(expression, nullptr);
}

bool Rule::contains_alternation() {
    return find_all<Alternation>([](const Alternation& alternation) -> bool {
        return alternation.size() > 1;
    }).size() > 0;
}

bool Rule::contains_expand() {
    return find_all<Expand>().size() > 0;
}

int Rule::count_terms() {
    return find_all<Term>().size();
}

int Rule::count_cc_tokens() {
    std::vector<CharacterClass*> ccs = find_all<CharacterClass>([](const CharacterClass& cc) -> bool {
        return !cc.any_char();
    });
    int count = 0;
    for (CharacterClass* cc: ccs) count += cc->token_count();
    return count;
}

void Rule::update_captures() {
    std::vector<Capture*> captures = find_all<Capture>();
    for (int i = 0; i < captures.size(); i++) {
        captures[i]->num = i+1;
    }
}
