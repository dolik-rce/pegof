#include "grammar.h"
#include "utils.h"
#include "log.h"

Grammar::Grammar(
    const std::vector<Directive>& directives,
    const std::vector<Rule>& rules,
    const Code& code
) : Node("Grammar", nullptr), directives(directives), rules(rules), code(code) {}

Grammar::Grammar(Parser& p) : Node("Grammar", nullptr), code("Code", this) {
    parse(p);
}

Grammar::Grammar(const std::string& s) : Node("Grammar", nullptr), code("Code", this) {
    Parser p(s);
    parse(p);
}

void Grammar::parse(Parser& p) {
    debug("Parsing Grammar");
    parse_comments(p);
    while (true) {
        Rule r(p, this);
        if (r) {
            rules.push_back(r);
            continue;
        }
        Directive d(p, this);
        if (d) {
            directives.push_back(d);
            continue;
        }
        code.parse(p);
        if (code) {
            break;
        }
        debug("Grammar parsed so far:\n%s", dump().c_str());
        error("Failed to parse grammar!");
    }
    update_parents();
    valid = true;
}

std::string join(const std::vector<std::string>& parts, const char* delimiter) {
    std::string result;
    for (int i = 0; i < parts.size(); i++) {
        result += (i ? delimiter : "") + parts[i];
    }
    return result;
}

std::string Grammar::to_string() const {
    std::vector<std::string> parts;
    std::string comments = format_comments();
    if (comments.size()) {
        parts.push_back(comments);
    }
    for (int i = 0; i < directives.size(); i++) {
        parts.push_back(directives[i].to_string());
    }
    for (int i = 0; i < rules.size(); i++) {
        parts.push_back(rules[i].to_string());
    }
    parts.push_back(code.to_string());
    std::string result = join(parts, "\n\n");
    if (!result.empty() && result.back() != '\n') {
        result += "\n";
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
    result += code.dump(indent + "  ") + "\n";
    return result;
}

Node* Grammar::operator[](int index) {
    if (index < directives.size()) {
        return &(directives[index]);
    } else if (index < directives.size() + rules.size()) {
        return &(rules[index - directives.size()]);
    } else {
        error("index out of bounds!");
    }
}

long Grammar::size() const {
    return directives.size() + rules.size();
}
