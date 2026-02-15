#include "ast/position.h"

#include "config.h"
#include "log.h"
#include "utils.h"

Position::Position(Node* parent): Node("Position", parent) {}
Position::Position(Parser& p, Node* parent): Node("Position", parent) {
    parse(p);
}

void Position::parse(Parser& p) {
    debug("Parsing Position");
    DebugIndent _;
    valid = p.match('^');
}

std::string Position::to_string(std::string indent) const {
    return "^";
}

std::string Position::dump(std::string indent) const {
    return indent + "POSITION";
}

bool Position::is_multiline() const {
    return false;
}

bool operator==(const Position& a, const Position& b) {
    return true;
}
bool operator!=(const Position& a, const Position& b) {
    return false;
}
