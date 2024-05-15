#include "ast/grammar.h"
#include "utils.h"
#include "log.h"

Grammar::Grammar(
    const std::vector<TopLevel>& nodes,
    const Code& code,
    const std::string& input_file
) : Node("Grammar", nullptr), nodes(nodes), code(code), input_file(input_file) {}

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
    debug("Parsing comments for node of type Grammar");
    while (p.match_comment()) {
        comments.push_back(p.last_match);
        debug("Comment: '%s'", comments.back().c_str());
    }

    while (true) {
        Rule r(p, this);
        if (r) {
            nodes.push_back(r);
            continue;
        }
        Directive d(p, this);
        if (d) {
            if (d.is_import() && !Config::get<bool>("no-follow") && Config::get(O_ALL)) {
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
                        nodes.push_back(r);
                        continue;
                    }
                } while (false);
                log(3, "Import done, returning to previous file.");
            } else {
                nodes.push_back(d);
            }
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

std::string to_string(const TopLevel& node) {
    switch(node.index()) {
    case 1: return std::get_if<Directive>(&node)->as<Directive>()->to_string();
    case 2: return std::get_if<Rule>(&node)->as<Rule>()->to_string();
    default:
        error("unsupporrted type!");
    }
}

std::string dump(const TopLevel& node, std::string indent) {
    switch(node.index()) {
    case 1: return std::get_if<Directive>(&node)->as<Directive>()->dump(indent);
    case 2: return std::get_if<Rule>(&node)->as<Rule>()->dump(indent);
    default:
        error("unsupporrted type!");
    }
}

std::string Grammar::to_string(std::string indent) const {
    std::vector<std::string> parts;
    std::string comments = format_comments();
    if (comments.size()) {
        parts.push_back(comments);
    }
    for (const TopLevel& node : nodes) {
        parts.push_back(::to_string(node));
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
    for (const TopLevel& node : nodes) {
        result += ::dump(node, indent + "  ") + "\n";
    }
    if (!code.empty()) {
        result += code.dump(indent + "  ") + "\n";
    }
    return result;
}

bool Grammar::is_multiline() const {
    return true;
}

Node* Grammar::operator[](int index) {
    if (index >= nodes.size()) {
        error("index out of bounds!");
    }
    TopLevel& n = nodes[index];
    switch(n.index()) {
    case 1: return std::get_if<Directive>(&n)->as<Directive>();
    case 2: return std::get_if<Rule>(&n)->as<Rule>();
    default:
        error("unsupporrted type!");
    }
}

long Grammar::size() const {
    return nodes.size();
}

void Grammar::erase(Rule* rule) {
    std::vector<TopLevel>::iterator it = std::find_if(nodes.begin(), nodes.end(), [rule](const TopLevel& n) {
        return std::get_if<Rule>(&n) == rule;
    });
    nodes.erase(it);
}
