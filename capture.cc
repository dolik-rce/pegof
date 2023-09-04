#include "capture.h"
#include "alternation.h"

Capture::Capture(const Alternation& expression) : expression(new Alternation(expression)) {}
Capture::Capture(Parser2& p) { parse(p); }

void Capture::parse(Parser2& p) {
    printf("parsing Capture\n");
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
    return "CAPTURE <" + expression->to_string() + ">";
}
