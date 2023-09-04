#include "sequence.h"

Sequence::Sequence(const std::vector<Term>& terms) : terms(terms) {}
Sequence::Sequence(Parser2& p) { parse(p); }

void Sequence::parse(Parser2& p) {
    printf("parsing Sequence\n");
    p.skip_space();
    Term t(p);
    if (!t) return;
    while (t) {
        terms.push_back(t);
        if (p.peek_re("\\S+\\s*<-")) break; // avoid parsing into the next rule
        t = Term(p);
    }
    valid = true;
}

std::string Sequence::to_string() const {
    std::string result = "SEQ " + terms[0].to_string();
    for (int i = 1; i < terms.size(); i++) {
        result += " " + terms[i].to_string();
    }
    return result;
}
