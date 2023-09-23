#include "grammar.h"
#include "utils.h"

Grammar::Grammar(
    const std::vector<Directive>& directives,
    const std::vector<Rule>& rules,
    const std::string& code
) : Node("Grammar", nullptr), directives(directives), rules(rules), code(code) {}

Grammar::Grammar(Parser& p) : Node("Grammar", nullptr) {
    parse(p);
}

Grammar::Grammar(const std::string& s) : Node("Grammar", nullptr) {
    Parser p(s);
    parse(p);
}

void Grammar::parse(Parser& p) {
    //~ printf("parsing  Grammar\n");
    parse_comments(p);
    while (true) {
        Directive d(p, this);
        if (d) {
            directives.push_back(d);
            continue;
        }
        Rule r(p, this);
        if (r) {
            rules.push_back(r);
            continue;
        }
        while (p.match_comment()) {
            code_comments.push_back(p.last_match);
            //~ printf("DBG: got code_comment '%s'\n", code_comments.back().c_str());
        }
        p.skip_space();
        if (p.match("%%")) {
            p.match_re("[\\s\\S]*");
            code = p.last_re_match.str(0);
            break;
        }
        if (p.is_eof()) break;
        printf("ERROR: Failed to parse grammar!\n");
        exit(1);
    }
    update_parents();
    valid = true; // !rules.empty();
}

std::string Grammar::to_string() const {
    std::string result = format_comments();
    if (!result.empty()) result += "\n";
    for (const Directive& directive : directives) {
        result += directive.to_string() + "\n";
    }
    if (!directives.empty())
        result += "\n";
    for (const Rule& rule : rules) {
        result += rule.to_string();
    }
    for (const std::string& comment : code_comments) {
        result += "# " + comment + "\n";
    }
    if (!code.empty()) {
        result += "%%\n" + code;
    }
    return result;
}

std::string Grammar::dump(std::string indent) const {
    std::string comments_info = " (" + std::to_string(comments.size()) + " comments)";
    std::string result = indent + "GRAMMAR" + comments_info + "\n";
    for (const Directive& directive : directives) {
        result += directive.dump(indent + "  ") + "\n";
    }
    for (const Rule& rule : rules) {
        result += rule.dump(indent + "  ") + "\n";
    }
    result += indent + "  CODE (" + std::to_string(code_comments.size()) + "): \"" + to_c_string(code) + "\"";
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
