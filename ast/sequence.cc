#include "sequence.h"
#include "log.h"

Sequence::Sequence(const std::vector<Term>& terms, Node* parent) : Node("Sequence", parent), terms(terms) {}
Sequence::Sequence(Parser& p, Node* parent) : Node("Sequence", parent) {
    parse(p);
}

void Sequence::parse(Parser& p) {
    debug("Parsing Sequence");
    DebugIndent _;
    Parser::State s = p.save_point();
    p.skip_space();
    Term t(p, this);
    if (!t) {
        s.rollback();
        return;
    }
    while (t) {
        terms.push_back(t);
        Parser::State peek = p.save_point();
        // Avoid parsing into the next rule (including its comments)
        parse_comments(p, false);
        if (p.peek_re("\\s*\\S+\\s*<-")) {
            peek.rollback();
            break;
        }
        peek.rollback();
        t = Term(p, this);
    }
    s.commit();
    valid = true;
}

std::string Sequence::to_string(std::string indent) const {
    std::string result = terms[0].to_string(indent);
    for (int i = 1; i < terms.size(); i++) {
        result += (terms[i].comments.empty() ? " " : "") + terms[i].to_string(indent);
    }
    return result;
}

std::string Sequence::dump(std::string indent) const {
    std::string result = indent + "SEQ\n";
    for (int i = 0; i < terms.size(); i++) {
        result += terms[i].dump(indent + "  ") + "\n";
    }
    return result;
}

bool Sequence::has_single_term() const {
    return terms.size() == 1;
}

Node* Sequence::operator[](int index) {
    if (index < terms.size()) {
        return &(terms[index]);
    } else {
        error("index out of bounds!");
    }
}

long Sequence::size() const {
    return terms.size();
}

bool operator==(const Sequence& a, const Sequence& b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        if (a.terms[i] != b.terms[i]) return false;
    }
    return true;
}

bool operator!=(const Sequence& a, const Sequence& b) {
    return !(a == b);
}
