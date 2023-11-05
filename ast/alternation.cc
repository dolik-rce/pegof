#include "alternation.h"
#include "rule.h"
#include "config.h"
#include "log.h"

Alternation::Alternation(const std::vector<Sequence>& sequences, Node* parent) : Node("Alternation", parent), sequences(sequences) {}
Alternation::Alternation(Parser& p, Node* parent) : Node("Alternation", parent) {
    parse(p);
}
Alternation::Alternation(Node* parent) : Node("Alternation", parent) {}

void Alternation::parse(Parser& p) {
    debug("Parsing Alternation");
    DebugIndent _;
    Parser::State sp = p.save_point();
    parse_comments(p);
    p.skip_space();
    Sequence s = Sequence(p, this);
    if (!s) {
        sp.rollback();
        return;
    }
    while (s) {
        sequences.push_back(s);
        if (!p.match("/")) {
            break;
        }
        s = Sequence(p, this);
    }
    parse_comments(p, true);
    sp.commit();
    valid = true;
}

std::string Alternation::to_string() const {
    bool multiline = parent->is<Rule>() && sequences.size() > Config::get<int>("wrap-limit");
    const char *delimiter = multiline ? "\n    / " : " / ";
    std::string result = sequences[0].to_string();
    for (int i = 1; i < sequences.size(); i++) {
        result += delimiter + sequences[i].to_string();
    }
    return result;
}

std::string Alternation::dump(std::string indent) const {
    std::string result = indent + "ALTERNATION" + dump_comments() + "\n";
    for (int i = 0; i < sequences.size(); i++) {
        result += sequences[i].dump(indent + "  ") + "\n";
    }
    return result;
}

Node* Alternation::operator[](int index) {
    if (index < sequences.size()) {
        return &(sequences[index]);
    } else {
        error("index out of bounds!");
    }
}

long Alternation::size() const {
    return sequences.size();
}

bool Alternation::has_single_term() const {
    return sequences.size() == 1 && sequences[0].has_single_term();
}

bool operator==(const Alternation& a, const Alternation& b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        if (a.sequences[i] != b.sequences[i]) return false;
    }
    return true;
}

bool operator!=(const Alternation& a, const Alternation& b) {
    return !(a == b);
}
