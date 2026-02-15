#include "ast/term.h"

#include "log.h"
#include "utils.h"

template<typename T, typename U = const Node> struct PrimaryVisitor {
    PrimaryVisitor(std::function<T(U&)> f): fn(f) {}
    std::function<T(U&)> fn;
    T operator()(U& x) {
        return fn(x);
    };
    T operator()(std::monostate) {
        error(INTERNAL_ERROR, "Calling function on empty Term!");
    };
};

Term::Term(
    char prefix, char quantifier, const Primary& primary, const std::optional<Action>& error_action, Node* parent
):
    Node("Term", parent), prefix(prefix), quantifier(quantifier), error_action(error_action), primary(primary) {}

Term::Term(Parser& p, Node* parent): Node("Term", parent), prefix(0), quantifier(0) {
    parse(p);
}

template<class T> bool Term::parse(Parser& p) {
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
    if (parse<Predicate>(p)) {
        s.commit();
        valid = true;
        parse_post_comment(p);
        return;
    }
    if (p.match_re("\\s*([!&])")) {
        prefix = p.last_re_match.str(1)[0];
    } else {
        prefix = 0;
    }
    if (!(parse<Reference>(p) || parse<String>(p) || parse<CharacterClass>(p) || parse<Position>(p) || parse<Expand>(p)
          || parse<Group>(p) || parse<Capture>(p) || parse<Action>(p))) {
        s.rollback();
        return;
    }
    if (p.match_re("\\s*([?*+])")) {
        quantifier = p.last_re_match.str(1)[0];
    } else {
        quantifier = 0;
    }
    if (p.match("~")) {
        error_action.emplace(p, this);
        if (!*error_action) {
            error_action.reset();
            s.rollback();
            return;
        }
    }
    parse_post_comment(p);
    s.commit();
    valid = true;
}

std::string Term::to_string(const Primary& x, const std::string& indent) const {
    return std::visit(PrimaryVisitor<std::string>([&indent](const Node& x) { return x.to_string(indent); }), primary);
}

std::string Term::dump(const Primary& x, std::string indent) const {
    return std::visit(PrimaryVisitor<std::string>([&indent](const Node& x) { return x.dump(indent); }), primary);
}

std::string Term::to_string(std::string indent) const {
    std::string result;
    if (comments.size()) {
        result += "\n" + format_comments(indent) + "\n" + indent;
    }
    if (prefix != 0) {
        result += std::string(1, prefix);
    }
    result += to_string(primary, indent);
    if (quantifier != 0) {
        result += std::string(1, quantifier);
    }
    if (error_action) {
        result += " ~ " + error_action->to_string();
    }
    if (!post_comment.empty()) {
        result += " #" + post_comment;
    }
    return result;
}

std::string Term::dump(std::string indent) const {
    std::string result = indent + "TERM";
    if (prefix != 0) {
        result += " " + std::string(1, prefix);
    }
    if (quantifier != 0) {
        result += " " + std::string(1, quantifier);
    }
    result += dump_comments() + "\n" + dump(primary, indent + "  ");
    if (error_action) {
        result += "\n" + error_action->dump(indent + "  ERROR ");
    }
    return result;
}

bool Term::is_multiline() const {
    if (!comments.empty()) {
        return true;
    }
    if (!post_comment.empty()) {
        return true;
    }
    return std::visit(PrimaryVisitor<bool>([](const Node& x) { return x.is_multiline(); }), primary);
}

Node* Term::operator[](int index) {
    if (index == 0) {
        return std::visit(PrimaryVisitor<Node*, Node>([](Node& x) { return &x; }), primary);
    } else {
        error(INTERNAL_ERROR, "index out of bounds!");
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
    return !(prefix || quantifier || error_action);
}

bool Term::is_negative() const {
    return prefix == '!';
}

bool Term::has_nonempty_error_action() const {
    return error_action && error_action->is_empty();
}

bool Term::error_action_contains_capture(int i) const {
    return error_action && error_action->contains_capture(0);
}

bool Term::error_action_contains_any_capture() const {
    return error_action && error_action->contains_any_capture();
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

void Term::remove_error_action() {
    error_action.reset();
}

bool operator==(const Term& a, const Term& b) {
    return a.prefix == b.prefix && a.quantifier == b.quantifier && a.primary == b.primary;
}

bool operator!=(const Term& a, const Term& b) {
    return !(a == b);
}
