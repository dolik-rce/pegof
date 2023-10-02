#include "term.h"
#include "log.h"

Term::Term(char prefix, char quantifier, const Primary& primary, Node* parent) : Node("Term", parent), prefix(prefix), quantifier(quantifier), primary(primary) {}
Term::Term(Parser& p, Node* parent) : Node("Term", parent) {
    parse(p);
}

template<class T>
bool Term::parse(Parser& p) {
    T parsed(p, this);
    if (parsed) {
        primary = parsed;
        return true;
    }
    return false;
}

void Term::parse(Parser& p) {
    debug("Parsing Term");
    p.skip_space();
    if (p.match('!') || p.match('&')) {
        prefix = p.last_match[0];
    } else {
        prefix = 0;
    }
    if (!(parse<Reference>(p)
          || parse<String>(p)
          || parse<CharacterClass>(p)
          || parse<Expand>(p)
          || parse<Group>(p)
          || parse<Action>(p))
    ) {
        return;
    }
    p.skip_space();
    if (p.match('?') || p.match('*') || p.match('+')) {
        quantifier = p.last_match[0];
    } else {
        quantifier = 0;
    }
    valid = true;
}

std::string Term::to_string(const Primary& x) const {
    switch(x.index()) {
    case 1: return std::get_if<String>(&x)->as<String>()->to_string();
    case 2: return std::get_if<Reference>(&x)->as<Reference>()->to_string();
    case 3: return std::get_if<CharacterClass>(&x)->as<CharacterClass>()->to_string();
    case 4: return std::get_if<Expand>(&x)->as<Expand>()->to_string();
    case 5: return std::get_if<Action>(&x)->as<Action>()->to_string();
    case 6: return std::get_if<Group>(&x)->as<Group>()->to_string();
    default:
        error("unsupporrted type!");
    }
}

std::string Term::dump(const Primary& x, std::string indent) const {
    switch(x.index()) {
    case 1: return std::get_if<String>(&x)->as<String>()->dump(indent);
    case 2: return std::get_if<Reference>(&x)->as<Reference>()->dump(indent);
    case 3: return std::get_if<CharacterClass>(&x)->as<CharacterClass>()->dump(indent);
    case 4: return std::get_if<Expand>(&x)->as<Expand>()->dump(indent);
    case 5: return std::get_if<Action>(&x)->as<Action>()->dump(indent);
    case 6: return std::get_if<Group>(&x)->as<Group>()->dump(indent);
    default:
        error("unsupporrted type!");
    }
}

std::string Term::to_string() const {
    std::string result;
    if (prefix != 0) result += std::string(1, prefix);
    result += to_string(primary);
    if (quantifier != 0) result += std::string(1, quantifier);
    return result;
}

std::string Term::dump(std::string indent) const {
    std::string result = indent + "TERM ";
    if (prefix != 0) result += std::string(1, prefix);
    if (quantifier != 0) result += std::string(1, quantifier);
    result += "\n" + dump(primary, indent + "  ");
    return result;
}

Node* Term::operator[](int index) {
    if (index == 0) {
        if (std::get_if<1>(&primary)) return (Node*)(std::get_if<1>(&primary));
        if (std::get_if<2>(&primary)) return (Node*)(std::get_if<2>(&primary));
        if (std::get_if<3>(&primary)) return (Node*)(std::get_if<3>(&primary));
        if (std::get_if<4>(&primary)) return (Node*)(std::get_if<4>(&primary));
        if (std::get_if<5>(&primary)) return (Node*)(std::get_if<5>(&primary));
        if (std::get_if<6>(&primary)) return (Node*)(std::get_if<6>(&primary));
        error("unsupported type!");
    } else {
        error("index out of bounds!");
    }
}

long Term::size() const {
    return 1;
}

bool Term::is_greedy() const {
    return quantifier == '*' || quantifier == '+';
}

bool Term::is_optional() const {
    return quantifier == '*' || quantifier == '?';
}

bool operator==(const Term& a, const Term& b) {
    return a.prefix == b.prefix && a.quantifier == b.quantifier && a.primary == b.primary;
}

bool operator!=(const Term& a, const Term& b) {
    return !(a == b);
}
