#include "ast/directive.h"

#include "log.h"
#include "utils.h"

Directive::Directive(const std::string& name, const std::string& value, Directive::Type type, Node* parent):
    Node("Directive", parent), name(name), value(value), type(type) {}
Directive::Directive(Parser& p, Node* parent): Node("Directive", parent) {
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
        if (p.match_code()) {
            value = trim(p.last_match);
        }
        type = CODE;
        valid = true;
    } else if (p.match_re("%(value|auxil|prefix|import)")) {
        name = p.last_re_match.str(1);
        if (p.match_string()) {
            value = trim(p.last_match);
        }
        type = STRING;
        valid = true;
    } else if (p.match("%marker")) {
        name = "marker";
        // TODO: improve parsing and formating
        if (p.match_re("@[a-zA-Z]\\S*(\\s+@[a-zA-Z]\\S*)*")) {
            value = trim(p.last_re_match.str(0));
        }
        type = MARKER;
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
    switch (type) {
    case CODE: result += " {"; break;
    case STRING: result += " \""; break;
    case MARKER: result += " "; break;
    }

    if (type == CODE && value[0] == '#') {
        // PackCC doesn't support single line C directives e.g.: %header { #include "x.h" }
        result += "\n    " + value + "\n";
    } else if (type == CODE) {
        std::vector<std::string> lines = split(value, "\\n");
        if (lines.size() > 1) {
            // multiline code
            result += "\n";
            std::string last_line = lines.back();
            for (int i = 0; i < last_line.size(); i++) {
                // copy indent from last line to the first line of result
                if (isspace(last_line[i])) {
                    result += last_line[i];
                } else {
                    break;
                }
            }
            result += value + "\n";
        } else {
            // single line code
            result += " " + value + " ";
        }
    } else {
        result += value;
    }
    if (type == CODE) {
        result += "}";
    } else if (type == STRING) {
        result += "\"";
    }
    if (!post_comment.empty()) {
        result += " #" + post_comment;
    }
    return result;
}

std::string Directive::dump(std::string indent) const {
    std::string comments_info = " (" + std::to_string(comments.size()) + " comments)";
    std::string result = indent + "DIRECTIVE " + name + comments_info;
    switch (type) {
    case CODE: result += " {" + to_c_string(value) + "}"; break;
    case STRING: result += " \"" + to_c_string(value) + "\""; break;
    case MARKER: result += to_c_string(value); break;
    }
    return result;
}

bool Directive::is_multiline() const {
    return !comments.empty() || type == CODE;
}
