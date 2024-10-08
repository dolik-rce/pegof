#include "ast/sequence.h"
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
    std::string result;
    for (int i = 0; i < terms.size(); i++) {
        if (i == 0) {
            // no delimiter before first term
        } else if (terms[i - 1].has_post_comment()) {
            result += "\n" + indent;
        } else if (!terms[i].has_comments()) {
            result += " ";
        }
        result += terms[i].to_string(indent);
    }
    return result;
}

std::string Sequence::dump(std::string indent) const {
    std::string result = indent + "SEQ\n";
    for (int i = 0; i < terms.size(); i++) {
        if (i > 0) result += "\n";
        result += terms[i].dump(indent + "  ");
    }
    return result;
}

bool Sequence::is_multiline() const {
    return std::any_of(terms.begin(), terms.end(), ::is_multiline);
}

bool Sequence::has_single_term() const {
    return terms.size() == 1;
}

const Term& Sequence::get_first_term() const {
    return terms[0];
}

Term& Sequence::get_first_term() {
    return terms[0];
}

Term& Sequence::get(int index) {
    return terms[index];
}

Node* Sequence::operator[](int index) {
    if (index < terms.size()) {
        return &(terms[index]);
    } else {
        error(INTERNAL_ERROR, "index out of bounds!");
    }
}

long Sequence::size() const {
    return terms.size();
}

void Sequence::insert(int index, const Sequence& s) {
    terms.insert(terms.begin() + index, s.terms.begin(), s.terms.end());
}

void Sequence::erase(Term* term) {
    std::vector<Term>::iterator it = std::find_if(terms.begin(), terms.end(), [term](const Term& t) {
        return &t == term;
    });
    terms.erase(it);
}

void Sequence::erase(int index) {
    terms.erase(terms.begin() + index);
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
