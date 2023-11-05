#include "reference.h"
#include "log.h"

Reference::Reference(const std::string& name, const std::string& var, Node* parent) : Node("Reference", parent), name(name), var(var) {}
Reference::Reference(Parser& p, Node* parent) : Node("Reference", parent) {
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
            error("expected identifier!\n");
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

bool operator==(const Reference& a, const Reference& b) {
    return a.name == b.name && a.var == b.var;
}

bool operator!=(const Reference& a, const Reference& b) {
    return !(a == b);
}
