#include "alternation.h"

Alternation::Alternation(const std::vector<Sequence>& sequences) : sequences(sequences) {}
Alternation::Alternation(Parser2& p) { parse(p); }
Alternation::Alternation() {}

void Alternation::parse(Parser2& p) {
    printf("parsing  Alternation\n");
    p.skip_space();
    Sequence s = Sequence(p);
    if (!s) return;
    while (s) {
        sequences.push_back(s);
        if (!p.match("/")) {
            break;
        }
        //~ printf("DBG: alt %s\n", p.last_match.c_str());
        s = Sequence(p);
    }
    valid = true;
}

std::string Alternation::to_string() const {
    std::string result = "ALT " + sequences[0].to_string();
    for (int i = 1; i < sequences.size(); i++) {
        result += " " + sequences[i].to_string();
    }
    return result;
}
