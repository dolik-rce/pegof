#include "ast/directive.h"
#include "utils.h"
#include "log.h"

Directive::Directive(const std::string& name, const std::string& value, bool code, Node* parent) : Node("Directive", parent), name(name), value(value), code(code) {}
Directive::Directive(Parser& p, Node* parent) : Node("Directive", parent) {
    parse(p);
}

bool Directive::is_import() const {
    return name == "import";
}

std::string Directive::get_value() const {
    return value;
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
    } else if (p.match_re("%(value|auxil|prefix|import)")) {
        name = p.last_re_match.str(1);
        if (p.match_string()) {
            value = trim(p.last_match);
        }
        code = false;
        valid = true;
    }
    parse_post_comment(p);
    if (!valid) {
        s.rollback();
    }
}

std::string Directive::to_string(std::string indent) const {
    std::string result = format_comments();
    if (result.size()) {
        result += "\n";
    }
    result += "%" + name;
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
    if (!post_comment.empty()) {
        result += " #" + post_comment;
    }
    return result;
}

std::string Directive::dump(std::string indent) const {
    std::string comments_info = " (" + std::to_string(comments.size()) + " comments)";
    std::string result = indent + "DIRECTIVE " + name + comments_info;
    result += (code ? " {": " \"" ) + to_c_string(value) + (code ? "}": "\"" );
    return result;
}

bool Directive::is_multiline() const {
    return !comments.empty() || code;
}
