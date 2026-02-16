#include "ast/predicate.h"

#include "log.h"
#include "utils.h"

Predicate::Predicate(Node* parent, std::string code, bool negative): Action(code, parent), negative(negative) {}
Predicate::Predicate(Parser& p, Node* parent): Action(parent) {
    parse(p);
}

void Predicate::parse(Parser& p) {
    debug("Parsing Predicate");
    DebugIndent _;
    if (!p.peek_re("[!&]\\{")) {
        return;
    }
    if (p.match('!')) {
        negative = true;
    } else {
        p.match('&');
        negative = false;
    }
    Action::parse(p);
}

std::string Predicate::to_string(std::string indent) const {
    if (is_multiline()) {
        return (negative ? "!{\n    " : "&{\n    ") + indent + replace(code, "\n", "\n    " + indent) + "\n" + indent
            + "}";
    } else {
        return (negative ? "!{ " : "&{ ") + code + " }";
    }
}

std::string Predicate::dump(std::string indent) const {
    return indent + "PREDICATE " + (negative ? "! " : "") + to_c_string(code);
}

const size_t PREDICATE_HASH = std::hash<const char*> {}("predicate");

size_t Predicate::hash() const {
    return combine(PREDICATE_HASH + negative, Action::hash());
}

bool operator==(const Predicate& a, const Predicate& b) {
    return a.code == b.code && a.negative == b.negative;
}
bool operator!=(const Predicate& a, const Predicate& b) {
    return !(a == b);
}
