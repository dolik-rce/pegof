#include "capture.h"
#include "alternation.h"

Capture::Capture(const Alternation& expression) : Node("Capture"), expression(new Alternation(expression)) {}
Capture::Capture(Parser2& p) : Node("Capture") { parse(p); }

void Capture::parse(Parser2& p) {
    //~ printf("parsing Capture\n");
    if (!p.match("<")) {
        return;
    }
    expression.reset(new Alternation(p));
    if (!*expression) {
        return;
    }
    if (!p.match(">")) {
        return;
    }
    valid = true;
}

std::string Capture::to_string() const {
    return "<" + expression->to_string() + ">";
}

std::string Capture::dump(std::string indent) const {
    return indent + "CAPTURE\n" + expression->dump(indent + "  ");
}

Node* Capture::operator[](int index) {
    if (index == 0) {
        return this;
    } else if (index == 1) {
        return (Node*)expression.get();
    } else {
        printf("ERROR: index out of bounds!\n");
        exit(1);
    }
}

long Capture::size() const {
    return 2;
}
