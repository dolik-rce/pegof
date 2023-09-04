#include "term.h"

Term::Term(char prefix, char quantifier, const Primary& primary) : prefix(prefix), quantifier(quantifier), primary(primary) {}
Term::Term(Parser2& p) { parse(p); }

template<class T>
bool Term::parse(Parser2& p) {
    T parsed(p);
    if (parsed) {
        primary = parsed;
        return true;
    }
    return false;
}

void Term::parse(Parser2& p) {
    printf("parsing  Term\n");

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

std::string Term::to_string() const {
    std::string result = "TERM ";
    if (prefix != 0) result += std::string(1, prefix);
    result += to_string(primary);
    if (quantifier != 0) result += std::string(1, quantifier);
    return result;
}
