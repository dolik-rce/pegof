#include "stringnode.h"
#include "utils.h"
#include "config.h"

String::String(const std::string& content) : Node("String"), content(content) {}
String::String(Parser2& p) : Node("String") { parse(p); }
String::String() : Node("String"), content("ERROR") {}

void String::parse(Parser2& p) {
    //~ printf("parsing  String\n");
    if (p.match_quoted("\"", "\"") || p.match_quoted("'", "'")) {
        content = p.last_match;
        valid = true;
    }
}

std::string String::to_string() const {
    if (Config::get<Config::QuoteType>("quotes") == Config::QT_SINGLE) {
        return '\'' + to_c_string(content, ESCAPE_SINGLE_QUOTES) + '\'';
    } else {
        return '"' + to_c_string(content, ESCAPE_DOUBLE_QUOTES) + '"';
    }
}

std::string String::dump(std::string indent) const {
    return indent + "STRING " + to_c_string(content);
}
