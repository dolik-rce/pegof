#include "stringnode.h"
#include "utils.h"
#include "config.h"
#include "log.h"

String::String(const std::string& content, Node* parent) : Node("String", parent), content(content) {}
String::String(Parser& p, Node* parent) : Node("String", parent) {
    parse(p);
}

void String::parse(Parser& p) {
    debug("Parsing String");
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

bool operator==(const String& a, const String& b) {
    return a.content == b.content;
}
bool operator!=(const String& a, const String& b) {
    return !(a == b);
}
