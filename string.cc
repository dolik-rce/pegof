#include "string.h"

String::String(const std::string& content) : content(content) {}
String::String(Parser2& p) { parse(p); }
String::String(): content("ERROR") {}

void String::parse(Parser2& p) {
    printf("parsing  String\n");
    if (p.match_quoted("\"", "\"") || p.match_quoted("'", "'")) {
        content = p.last_match;
        valid = true;
    }
}

std::string String::to_string() const {
    return "STRING " + content;
}
