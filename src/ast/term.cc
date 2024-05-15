#include "ast/term.h"
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
    DebugIndent _;
    Parser::State s = p.save_point();
    parse_comments(p);
    if (p.match_re("\\s*([!&])")) {
        prefix = p.last_re_match.str(1)[0];
    } else {
        prefix = 0;
    }
    if (!(parse<Reference>(p)
          || parse<String>(p)
          || parse<CharacterClass>(p)
          || parse<Expand>(p)
          || parse<Group>(p)
          || parse<Capture>(p)
          || parse<Action>(p))
    ) {
        s.rollback();
        return;
    }
    if (p.match_re("\\s*([?*+])")) {
        quantifier = p.last_re_match.str(1)[0];
    } else {
        quantifier = 0;
    }
    parse_post_comment(p);
    s.commit();
    valid = true;
}

std::string Term::to_string(const Primary& x, const std::string& indent) const {
    switch(x.index()) {
    case 1: return std::get_if<String>(&x)->as<String>()->to_string(indent);
    case 2: return std::get_if<Reference>(&x)->as<Reference>()->to_string(indent);
    case 3: return std::get_if<CharacterClass>(&x)->as<CharacterClass>()->to_string(indent);
    case 4: return std::get_if<Expand>(&x)->as<Expand>()->to_string(indent);
    case 5: return std::get_if<Action>(&x)->as<Action>()->to_string(indent);
    case 6: return std::get_if<Group>(&x)->as<Group>()->to_string(indent);
    case 7: return std::get_if<Capture>(&x)->as<Capture>()->to_string(indent);
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
    case 7: return std::get_if<Capture>(&x)->as<Capture>()->dump(indent);
    default:
        error("unsupporrted type!");
    }
}

std::string Term::to_string(std::string indent) const {
    std::string result;
    if (comments.size()) {
        result += "\n" + format_comments(indent) + "\n" + indent;
    }
    if (prefix != 0) result += std::string(1, prefix);
    result += to_string(primary, indent);
    if (quantifier != 0) result += std::string(1, quantifier);
    if (!post_comment.empty()) {
        result += " #" + post_comment;
    }
    return result;
}

std::string Term::dump(std::string indent) const {
    std::string result = indent + "TERM";
    if (prefix != 0) result += " " + std::string(1, prefix);
    if (quantifier != 0) result += " " + std::string(1, quantifier);
    result += dump_comments() + "\n" + dump(primary, indent + "  ");
    return result;
}

bool Term::is_multiline() const {
    if (!comments.empty()) return true;
    if (!post_comment.empty()) return true;
    if (std::get_if<1>(&primary)) return (Node*)(std::get_if<1>(&primary))->is_multiline();
    if (std::get_if<2>(&primary)) return (Node*)(std::get_if<2>(&primary))->is_multiline();
    if (std::get_if<3>(&primary)) return (Node*)(std::get_if<3>(&primary))->is_multiline();
    if (std::get_if<4>(&primary)) return (Node*)(std::get_if<4>(&primary))->is_multiline();
    if (std::get_if<5>(&primary)) return (Node*)(std::get_if<5>(&primary))->is_multiline();
    if (std::get_if<6>(&primary)) return (Node*)(std::get_if<6>(&primary))->is_multiline();
    if (std::get_if<7>(&primary)) return (Node*)(std::get_if<7>(&primary))->is_multiline();
    error("unsupported type!");
}

Node* Term::operator[](int index) {
    if (index == 0) {
        if (std::get_if<1>(&primary)) return (Node*)(std::get_if<1>(&primary));
        if (std::get_if<2>(&primary)) return (Node*)(std::get_if<2>(&primary));
        if (std::get_if<3>(&primary)) return (Node*)(std::get_if<3>(&primary));
        if (std::get_if<4>(&primary)) return (Node*)(std::get_if<4>(&primary));
        if (std::get_if<5>(&primary)) return (Node*)(std::get_if<5>(&primary));
        if (std::get_if<6>(&primary)) return (Node*)(std::get_if<6>(&primary));
        if (std::get_if<7>(&primary)) return (Node*)(std::get_if<7>(&primary));
        error("unsupported type!");
    } else {
        error("index out of bounds!");
    }
}

long Term::size() const {
    return 1;
}

bool Term::is_quantified() const {
    return quantifier != 0;
}
bool Term::is_prefixed() const {
    return prefix != 0;
}

bool Term::is_greedy() const {
    return quantifier == '*' || quantifier == '+';
}

bool Term::is_optional() const {
    return quantifier == '*' || quantifier == '?';
}

bool Term::is_simple() const {
    return !prefix && !quantifier;
}

bool Term::is_negative() const {
    return prefix == '!';
}

void Term::flip_negation() {
    prefix = is_negative() ? 0 : '!';
}

void Term::set_prefix(int new_prefix) {
    prefix = new_prefix;
}

void Term::set_quantifier(int new_quantifier) {
    quantifier = new_quantifier;
}

void Term::set_content(Primary content) {
    primary = content;
}

bool Term::same_prefix(const Term& t) {
    return prefix == t.prefix;
}

bool Term::same_quantifier(const Term& t) {
    return quantifier == t.quantifier;
}

void Term::copy_prefix(const Term& other) {
    prefix = other.prefix;
}

void Term::copy_quantifier(const Term& other) {
    quantifier = other.quantifier;
}

void Term::copy_content(const Term& other) {
    primary = other.primary;
}

bool operator==(const Term& a, const Term& b) {
    return a.prefix == b.prefix && a.quantifier == b.quantifier && a.primary == b.primary;
}

bool operator!=(const Term& a, const Term& b) {
    return !(a == b);
}
