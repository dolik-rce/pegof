#include "term.h"

Term::Term(char prefix, char quantifier, const Primary& primary, Node* parent) : Node("Term", parent), prefix(prefix), quantifier(quantifier), primary(primary) {}
Term::Term(Parser2& p, Node* parent) : Node("Term", parent) {
    parse(p);
}

template<class T>
bool Term::parse(Parser2& p) {
    T parsed(p, this);
    if (parsed) {
        primary = parsed;
        return true;
    }
    return false;
}

void Term::parse(Parser2& p) {
    //~ printf("parsing  Term\n");
    p.skip_space();
    if (p.match('!') || p.match('&')) {
        prefix = p.last_match[0];
    } else {
        prefix = 0;
    }
    if (!(parse<Reference>(p)
          || parse<String>(p)
          || parse<CharacterClass2>(p)
          || parse<Expand>(p)
          || parse<Group>(p)
          || parse<Capture>(p)
          || parse<Action>(p))
    ) {
        return;
    }
    if (p.match('?') || p.match('*') || p.match('+')) {
        quantifier = p.last_match[0];
    } else {
        quantifier = 0;
    }
    valid = true;
}

std::string Term::to_string(const Primary& x) const {
    return std::visit([](const auto& value) -> std::string {
        return value.to_string();
    }, x);
}

std::string Term::dump(const Primary& x, std::string indent) const {
    return std::visit([indent](const auto& value) -> std::string {
        return value.dump(indent);
    }, x);
}

std::string Term::to_string() const {
    std::string result;
    if (prefix != 0) result += std::string(1, prefix);
    result += to_string(primary);
    if (quantifier != 0) result += std::string(1, quantifier);
    return result;
}

std::string Term::dump(std::string indent) const {
    std::string result = indent  + "TERM ";
    if (prefix != 0) result += std::string(1, prefix);
    if (quantifier != 0) result += std::string(1, quantifier);
    result += "\n" + dump(primary, indent + "  ");
    return result;
}

Node* Term::operator[](int index) {
    if (index == 0) {
        return this;
    } else if (index == 1) {
        if (std::get_if<0>(&primary)) return (Node*)(std::get_if<0>(&primary));
        if (std::get_if<1>(&primary)) return (Node*)(std::get_if<1>(&primary));
        if (std::get_if<2>(&primary)) return (Node*)(std::get_if<2>(&primary));
        if (std::get_if<3>(&primary)) return (Node*)(std::get_if<3>(&primary));
        if (std::get_if<4>(&primary)) return (Node*)(std::get_if<4>(&primary));
        if (std::get_if<5>(&primary)) return (Node*)(std::get_if<5>(&primary));
        if (std::get_if<6>(&primary)) return (Node*)(std::get_if<6>(&primary));
        printf("ERROR: unsupported type\n");
        exit(1);
    } else {
        printf("ERROR: index out of bounds!\n");
        exit(1);
    }
}

long Term::size() const {
    return 2;
}
