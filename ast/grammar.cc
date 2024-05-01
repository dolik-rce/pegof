#include "ast/grammar.h"
#include "utils.h"
#include "log.h"

Grammar::Grammar(
    const std::vector<Directive>& directives,
    const std::vector<Rule>& rules,
    const Code& code,
    const std::string& input_file
) : Node("Grammar", nullptr), directives(directives), rules(rules), code(code), input_file(input_file) {}

Grammar::Grammar(Parser& p, const std::string& input_file)
    : Node("Grammar", nullptr), code("", this), input_file(input_file)
{
    parse(p);
}

Grammar::Grammar(const std::string& s, const std::string& input_file)
    : Node("Grammar", nullptr), code("", this), input_file(input_file)
{
    Parser p(s);
    parse(p);
}

void Grammar::parse(Parser& p) {
    debug("Parsing Grammar");
    DebugIndent _;
    //~ parse_comments(p);
    debug("Parsing comments for node of type Grammar");
    while (p.match_comment()) {
        comments.push_back(p.last_match);
        debug("Comment: '%s'", comments.back().c_str());
    }

    while (true) {
        Rule r(p, this);
        if (r) {
            rules.push_back(r);
            continue;
        }
        Directive d(p, this);
        if (d) {
            if (d.is_import() && Config::get(O_ALL)) {
                std::string name = d.get_value();
                std::string path = name.substr(0, 1) != "/"
                    ? find_file(name, Config::get_all_imports_dirs(input_file))
                    : name;
                if (path.empty()) {
                    error("File '%s' not found", d.get_value().c_str());
                }
                log(1, "Importing file '%s'...", path.c_str());
                Parser parser(read_file(path));
                do {
                    Rule r(parser, this);
                    if (r) {
                        rules.push_back(r);
                        continue;
                    }
                } while (false);
                log(3, "Import done, returning to previous file.");
            } else {
                directives.push_back(d);
            }
            debug("DBG: Grammar parsed so far:\n%s", dump().c_str());
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

std::string Grammar::to_string(std::string indent) const {
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
    if (!code.empty()) {
        parts.push_back(code.to_string());
    }
    std::string result = join(parts, "\n\n");
    if (!result.empty() && result.back() != '\n') {
        result += "\n";
    }
    return result;
}

std::string Grammar::dump(std::string indent) const {
    std::string result = indent + "GRAMMAR";
    if (!comments.empty()) {
        result += " (" + std::to_string(comments.size()) + " comments)";
    }
    result += "\n";
    for (const Directive& directive : directives) {
        result += directive.dump(indent + "  ") + "\n";
    }
    for (const Rule& rule : rules) {
        result += rule.dump(indent + "  ") + "\n";
    }
    if (!code.empty()) {
        result += code.dump(indent + "  ") + "\n";
    }
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

void Grammar::erase(Rule* rule) {
    std::vector<Rule>::iterator it = std::find_if(rules.begin(), rules.end(), [rule](const Rule& r) {
        return &r == rule;
    });
    rules.erase(it);
}
