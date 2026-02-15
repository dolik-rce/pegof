#include "ast/reference.h"

#include "ast/rule.h"
#include "log.h"

Reference::Reference(const std::string& name, const std::string& var, Node* parent):
    Node("Reference", parent), name(name), var(var) {}
Reference::Reference(Parser& p, Node* parent): Node("Reference", parent) {
    parse(p);
}

void Reference::parse(Parser& p) {
    debug("Parsing Reference");
    DebugIndent _;
    if (!p.match_identifier()) {
        return;
    }
    std::string p1 = p.last_match;
    if (p.match(":")) {
        if (!p.match_identifier()) {
            error(PARSING_ERROR, "expected identifier!\n");
        }
        std::string p2 = p.last_match;
        name = p2;
        var = p1;
    } else {
        name = p1;
        var = "";
    }
    valid = true;
}

std::string Reference::to_string(std::string indent) const {
    if (!var.empty()) {
        return var + ":" + name;
    }
    return name;
}

std::string Reference::dump(std::string indent) const {
    std::string result = indent + "REF ";
    if (!var.empty()) {
        result += var + ":";
    }
    result += name;
    return result;
}

bool Reference::is_multiline() const {
    return false;
}

std::string Reference::get_name() const {
    return name;
}

bool Reference::references(const Rule* rule) const {
    return name == rule->name;
}

bool Reference::has_variable() const {
    return !var.empty();
}

void Reference::remove_variable() {
    return var.clear();
}

bool operator==(const Reference& a, const Reference& b) {
    return a.name == b.name && a.var == b.var;
}

bool operator!=(const Reference& a, const Reference& b) {
    return !(a == b);
}
