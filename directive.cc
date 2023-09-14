#include "directive.h"
#include "utils.h"

Directive::Directive(const std::string& name, const std::string& value, bool code, Node* parent): Node("Directive", parent), name(name), value(value), code(code) {}
Directive::Directive(Parser2& p, Node* parent) : Node("Directive", parent) { parse(p); }

void Directive::parse(Parser2& p) {
    parse_comments(p);
    //~ printf("parsing  Directive\n");
    if (p.match_re("%(earlysource|earlycommon|earlyheader|source|header|common)")) {
        name = p.last_re_match.str(1);
        if(p.match_code()) {
            value = p.last_match;
        }
        code = true;
        valid = true;
    } else if (p.match_re("%(value|auxil|prefix)")) {
        name = p.last_re_match.str(1);
        if (p.match_quoted("\"", "\"") || p.match_quoted("'", "'")) {
            value = p.last_match;
            //~ printf("value=%s\n", value.c_str());
        }
        code = false;
        valid = true;
    }
}

std::string Directive::to_string() const {
    std::string result = comments() + "%" + name;
    if (code) {
        result += " {" + value + "}\n";
    } else {
        result += " \"" + value + "\"\n";
    }
    return result;
}

std::string Directive::dump(std::string indent) const {
    std::string result = indent + "DIRECTIVE " + name;
    result += (code ? " {": " \"" ) + to_c_string(value) + (code ? "}": "\"" );
    return result;
}
