#include "ast/marker.h"

#include "config.h"
#include "log.h"
#include "utils.h"

Marker::Marker(const std::string& name, Node* parent): Node("Marker", parent), name(name) {}
Marker::Marker(Parser& p, Node* parent): Node("Marker", parent) {
    parse(p);
}

void Marker::parse(Parser& p) {
    debug("Parsing Marker");
    DebugIndent _;
    Parser::State s = p.save_point();
    if (p.match('@') && p.match_identifier()) {
        name = p.last_re_match.str(0);
        s.commit();
        valid = true;
    } else {
        s.rollback();
    }
}

std::string Marker::to_string(std::string indent) const {
    return "@" + name;
}

std::string Marker::dump(std::string indent) const {
    return indent + "MARKER " + name;
}

bool Marker::is_multiline() const {
    return false;
}

const size_t MARKER_HASH = std::hash<const char*> {}("marker");

size_t Marker::hash() const {
    return combine(MARKER_HASH, name);
}

bool operator==(const Marker& a, const Marker& b) {
    return a.name == b.name;
}
bool operator!=(const Marker& a, const Marker& b) {
    return !(a == b);
}
