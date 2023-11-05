#include "directive.h"
#include "utils.h"
#include "log.h"

Directive::Directive(const std::string& name, const std::string& value, bool code, Node* parent) : Node("Directive", parent), name(name), value(value), code(code) {}
Directive::Directive(Parser& p, Node* parent) : Node("Directive", parent) {
    parse(p);
}

void Directive::parse(Parser& p) {
    debug("Parsing Directive");
    DebugIndent _;
    Parser::State s = p.save_point();
    parse_comments(p);
    if (p.match_re("%(earlysource|earlycommon|earlyheader|source|header|common)")) {
        name = p.last_re_match.str(1);
        if(p.match_code()) {
            value = trim(p.last_match);
        }
        code = true;
        valid = true;
    } else if (p.match_re("%(value|auxil|prefix)")) {
        name = p.last_re_match.str(1);
        if (p.match_quoted("\"", "\"") || p.match_quoted("'", "'")) {
            value = trim(p.last_match);
        }
        code = false;
        valid = true;
    }
    if (!valid) {
        s.rollback();
    }
}

std::string Directive::to_string(std::string indent) const {
    std::string result = format_comments() + "%" + name;
    result += code ? " {"  : " \"";

    if (code && value[0] == '#') {
        // PackCC doesn't support single line C directives e.g.: %header { #include "x.h" }
        result += "\n    " + value + "\n";
    } else if (code) {
        std::vector<std::string> lines = split(value, "\\n");
        if (lines.size() > 1) {
            // multiline code
            result += "\n";
            std::string last_line = lines.back();
            for (int i = 0; i < last_line.size(); i++) {
                // copy indent from last line to the first line of result
                if (isspace(last_line[i])) result += last_line[i];
                else break;
            }
            result += value + "\n";
        } else {
            // single line code
            result += " " + value + " ";
        }
    } else {
        result += value;
    }
    result += code ? "}"  : "\"";
    return result;
}

std::string Directive::dump(std::string indent) const {
    std::string comments_info = " (" + std::to_string(comments.size()) + " comments)";
    std::string result = indent + "DIRECTIVE " + name + comments_info;
    result += (code ? " {": " \"" ) + to_c_string(value) + (code ? "}": "\"" );
    return result;
}
