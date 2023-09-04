#include "directive.h"

Directive::Directive(const std::string& name, const std::string& value, bool code): name(name), value(value), code(code) {}
Directive::Directive(Parser2& p) { parse(p); }

void Directive::parse(Parser2& p) {
    printf("parsing  Directive\n");
    std::string name;
    std::string value;
    if (p.match_re("%(earlysource|earlycommon|earlyheader|source|header|common)")) {
        name = p.last_re_match.str(1);
        if(p.match_code()) {
            value = p.last_match;
        }
        code = true;
        valid = true;
    } else if (p.match_re("%(value|auxil|prefix)")) {
        name = p.last_re_match.str(1);
        if (p.match_quoted("\"", "\"")) {
            value = p.last_match;
        }
        code = false;
        valid = true;
    }
}

std::string Directive::to_string() const {
    std::string result = "DIRECTIVE %" + name;
    if (code) {
        result += " " + value + "\n";
    } else {
        result += " " + value + "\n";
    }
    return result;
}
