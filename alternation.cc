#include "alternation.h"
#include "config.h"

Alternation::Alternation(const std::vector<Sequence>& sequences, Node* parent) : Node("Alternation", parent), sequences(sequences) {}
Alternation::Alternation(Parser2& p, Node* parent) : Node("Alternation", parent) {
    parse(p);
}
Alternation::Alternation() : Node("Alternation", nullptr) {}

void Alternation::parse(Parser2& p) {
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
    int multiline = sequences.size() > Config::get<int>("wrap-limit");
    const char *delimiter = multiline ? "\n    / " : " / ";
    std::string result = sequences[0].to_string();
    for (int i = 1; i < sequences.size(); i++) {
        result += delimiter + sequences[i].to_string();
    }
    return result;
}

std::string Alternation::dump(std::string indent) const {
    std::string result = indent + "ALTERNATION\n";
    for (int i = 0; i < sequences.size(); i++) {
        result += sequences[i].dump(indent + "  ") + "\n";
    }
    return result;
}

Node* Alternation::operator[](int index) {
    if (index == 0) {
        return this;
    } else if (index <= sequences.size()) {
        return &(sequences[index - 1]);
    } else {
        printf("ERROR: index out of bounds!\n");
        exit(1);
    }
}

long Alternation::size() const {
    return 1 + sequences.size();
}
