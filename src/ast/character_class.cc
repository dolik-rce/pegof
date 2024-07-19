#include "ast/character_class.h"

#include "log.h"
#include "packcc_wrapper.h"
#include "utils.h"

#include <cstring>
#include <iomanip>
#include <numeric>
#include <sstream>

CharacterClass::CharacterClass(const std::string& content, Node* parent):
    Node("CharacterClass", parent), content(content), dash(false), negation(false) {
    Parser p(content);
    parse(p);
}
CharacterClass::CharacterClass(Parser& p, Node* parent): Node("CharacterClass", parent), dash(false), negation(false) {
    parse(p);
}

static std::string to_char(int c) {
    if (c >= 0x80 && c <= 0xffff) {
        return "\\u" + to_hex(c, 4);
    }
    if (c > 0xffff) {
        // Calculate surrogate pairs for characters beyond BMP
        int adjusted = c - 0x10000;
        int high = (adjusted >> 10) + 0xD800;
        int low = (adjusted & 0x3FF) + 0xDC00;
        return "\\u" + to_hex(high, 4) + "\\u" + to_hex(low, 4);
    }
    switch (c) {
    case '\r': return R"(\r)";
    case '\n': return R"(\n)";
    case '\t': return R"(\t)";
    case '\v': return R"(\v)";
    case '\f': return R"(\f)";
    case '[': return R"(\[)";
    case ']': return R"(\])";
    case '\\': return R"(\\)";
    default: return (c < 0x20) ? ("\\x" + to_hex(c, 2)) : std::string(1, (char)c);
    }
}

static int get_char(const std::string& s, int& pos) {
    int result;
    pos += pcc_utf8_to_utf32(s.c_str() + pos, &result);
    if (result != '\\' || pos == s.size()) {
        return result;
    }
    switch (s[pos]) {
    case '[': pos++; return '[';
    case ']': pos++; return ']';
    case '^': pos++; return '^';
    default: return result;
    }
}

void CharacterClass::parse_content(const std::string& str) {
    Parser p(str);
    negation = p.match('^');
    dash = p.match('-');
    content = str.substr(negation + dash);
    tokenize();
    update_content();
}

void CharacterClass::tokenize() {
    tokens.clear();
    int pos = 0;
    while (pos < content.size()) {
        Token result;
        result.first = get_char(content, pos);
        if (content[pos] == '-') {
            pos++;
            result.second = get_char(content, pos);
        } else {
            result.second = result.first;
        }
        tokens.push_back(result);
    }
}

void CharacterClass::parse(Parser& p) {
    debug("Parsing CharacterClass");
    DebugIndent _;
    Parser::State s = p.save_point();
    p.skip_space();
    if (p.match('.')) {
        content = ".";
    } else if (p.peek('[')) {
        std::string tmp;
        pcc_match_quoted(&p, &tmp);
        parse_content(tmp);
    } else {
        s.rollback();
        return;
    }
    s.commit();
    valid = true;
}

void CharacterClass::update_content() {
    if (any_char()) {
        return;
    }

    content = "";

    for (size_t i = 0; i < tokens.size(); i++) {
        size_t size = tokens[i].second - tokens[i].first;
        std::string first = (i == 0 && tokens[i].first == '^') ? "\\^" : to_char(tokens[i].first);
        std::string second = to_char(tokens[i].second);
        switch (size) {
        case 0: content += first; break;
        case 1: content += first + second; break;
        default: content += first + '-' + second; break;
        }
    }
}

std::string CharacterClass::to_string(std::string indent) const {
    if (any_char()) {
        return ".";
    } else {
        std::string result = "[";
        if (negation) {
            result += '^';
        }
        if (dash) {
            result += '-';
        }
        result += content + ']';
        return result;
    }
}

std::string CharacterClass::dump(std::string indent) const {
    return indent + "CHAR_CLASS " + content;
}

bool CharacterClass::is_multiline() const {
    return false;
}

bool CharacterClass::normalize() {
    if (tokens.empty()) {
        return false;
    }
    std::string original = content;
    sort(tokens.begin(), tokens.end());
    Tokens init(1, tokens[0]);
    tokens = accumulate(tokens.begin() + 1, tokens.end(), init, [](Tokens& acc, const Token& item) {
        Token& prev = acc.back();
        if (prev.second + 1 < item.first) {
            acc.push_back(item);
        } else if (item.second > prev.second) {
            prev.second = item.second;
        }
        return acc;
    });
    update_content();
    return content != original;
}

void CharacterClass::flip_negation() {
    negation = !negation;
}

bool CharacterClass::any_char() const {
    return content == ".";
}

int CharacterClass::token_count() const {
    return tokens.size();
}

bool CharacterClass::is_single_char() const {
    int size = content.size() + (dash ? 1 : 0);
    return size == 1;
}

bool CharacterClass::is_negative() const {
    return negation;
}

String CharacterClass::convert_to_string() const {
    return String(dash ? "-" : content, parent);
}

void CharacterClass::merge(const CharacterClass& cc) {
    dash |= cc.dash;
    content += cc.content;
    tokenize();
    update_content();
}

bool operator==(const CharacterClass& a, const CharacterClass& b) {
    return a.negation == b.negation && a.dash == b.dash && a.content == b.content;
}

bool operator!=(const CharacterClass& a, const CharacterClass& b) {
    return !(a == b);
}
