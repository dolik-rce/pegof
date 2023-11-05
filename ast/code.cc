#include "code.h"
#include "config.h"
#include "utils.h"
#include "log.h"

Code::Code(const std::string& content, Node* parent) : Node("Code", parent), content(content) {}
Code::Code(Parser& p, Node* parent) : Node("Code", parent) {
    parse(p);
}

void Code::parse(Parser& p) {
    debug("Parsing Code");
    DebugIndent _;
    Parser::State s = p.save_point();
    parse_comments(p);
    p.skip_space();
    if (p.is_eof()) {
        valid = true;
        s.commit();
        return;
    }
    if (p.match("%%")) {
        p.match_re("[\\s\\S]*");
        content = p.last_re_match.str(0);
        s.commit();
        valid = true;
        return;
    }
    s.rollback();
}

std::string Code::to_string() const {
    std::string result = format_comments() + (comments.empty() ? "" : "\n");
    if (!content.empty()) result += "%%\n";
    result += content;
    return result;
}

std::string Code::dump(std::string indent) const {
    return indent + "CODE " + dump_comments() + " \"" + to_c_string(content) + "\"";
}
