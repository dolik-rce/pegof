#include "directive.h"
#include "utils.h"
#include "log.h"

Directive::Directive(const std::string& name, const std::string& value, bool code, Node* parent) : Node("Directive", parent), name(name), value(value), code(code) {}
Directive::Directive(Parser& p, Node* parent) : Node("Directive", parent) {
    parse(p);
}

void Directive::parse(Parser& p) {
    debug("Parsing Directive");
    Parser::State s = p.save_point();
    parse_comments(p);
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
        }
        code = false;
        valid = true;
    }
    if (!valid) {
        s.rollback();
    }
}

std::string Directive::to_string() const {
    std::string result = format_comments() + "%" + name;
    if (code) {
        result += " {" + value + "}";
    } else {
        result += " \"" + value + "\"";
    }
    return result;
}

std::string Directive::dump(std::string indent) const {
    std::string comments_info = " (" + std::to_string(comments.size()) + " comments)";
    std::string result = indent + "DIRECTIVE " + name + comments_info;
    result += (code ? " {": " \"" ) + to_c_string(value) + (code ? "}": "\"" );
    return result;
}
