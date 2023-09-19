#include "sequence.h"

Sequence::Sequence(const std::vector<Term>& terms) : Node("Sequence", this), terms(terms) {}
Sequence::Sequence(Parser2& p) : Node("Sequence", this) {
    parse(p);
}

void Sequence::parse(Parser2& p) {
    //~ printf("parsing Sequence\n");
    p.skip_space();
    Term t(p, this);
    if (!t) return;
    while (t) {
        terms.push_back(t);
        if (p.peek_re("\\S+\\s*<-")) break; // avoid parsing into the next rule
        t = Term(p, this);
    }
    valid = true;
}

std::string Sequence::to_string() const {
    std::string result = terms[0].to_string();
    for (int i = 1; i < terms.size(); i++) {
        result += " " + terms[i].to_string();
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

Node* Sequence::operator[](int index) {
    if (index == 0) {
        return this;
    } else if (index <= terms.size()) {
        return &(terms[index - 1]);
    } else {
        printf("ERROR: index out of bounds!\n");
        exit(1);
    }
}

long Sequence::size() const {
    return 1 + terms.size();
}
