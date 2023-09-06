#include "character_class.h"

CharacterClass2::CharacterClass2(const std::string& content) : Node("CharacterClass"), content(content) {}
CharacterClass2::CharacterClass2(Parser2& p) : Node("CharacterClass") { parse(p); }

void CharacterClass2::parse(Parser2& p) {
    //~ printf("parsing CharacterClass2\n");
    p.skip_space();
    if (p.match('.')) {
        content = ".";
    } else if (p.match_quoted("[", "]")) {
        content = p.last_match;
    } else {
        return;
    }
    valid = true;
}

std::string CharacterClass2::to_string() const {
    return content;
}

std::string CharacterClass2::dump(std::string indent) const {
    return indent + "CHAR_CLASS " + content;
}
