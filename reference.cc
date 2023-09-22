#include "reference.h"

Reference::Reference(const std::string& name, const std::string& var, Node* parent) : Node("Reference", parent), name(name), var(var) {}
Reference::Reference(Parser& p, Node* parent) : Node("Reference", parent) {
    parse(p);
}

void Reference::parse(Parser& p) {
    //~ printf("parsing  Reference\n");
    if (!p.match_identifier()) {
        return;
    }
    std::string p1 = p.last_match;
    if (p.match(":")) {
        if (!p.match_identifier()) {
            printf("ERROR: expected identifier!\n");
            exit(1);
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

std::string Reference::to_string() const {
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
