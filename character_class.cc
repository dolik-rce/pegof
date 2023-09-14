#include "character_class.h"

#include <sstream>
#include <iomanip>
#include <numeric>

CharacterClass2::CharacterClass2(const std::string& content, Node* parent) : Node("CharacterClass", parent), content(content) { parseContent(); }
CharacterClass2::CharacterClass2(Parser2& p, Node* parent) : Node("CharacterClass", parent) { parse(p); }

static std::string to_char(int c) {
    if (c > 127) {
        std::stringstream ss;
        ss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << c;
        return ss.str();
    }
    switch (c) {
    case '\r': return "\\r";
    case '\n': return "\\n";
    case '\t': return "\\t";
    case '\v': return "\\v";
    case '[': return "\\[";
    case ']': return "\\]";
    case '^': return "\\^";
    case '-': return "\\-";
    case '\\': return "\\\\";
    default: return std::string(1, (char)c);
    }
}

static int get_char(Parser2& p) {
    if (p.match('\\')) {
        switch (p.current()) {
        case 'r': return '\r';
        case 'n': return '\n';
        case 't': return '\t';
        case 'v': return '\v';
        case '[': return '[';
        case ']': return ']';
        case '^': return '^';
        case '-': return '-';
        case '\\': return '\\';
        case 'u':
            if (!p.match_re("\\d\\d\\d\\d")) {
                printf("ERROR: Invalid unicode char in character class!\n");
            }
            std::string code = p.last_re_match.str(0);
            return std::stoi(code.c_str(), nullptr, 16);
        }
    }
    if (!p.match_any()) {
        return -1; // this should never happen for well-formed charracter classes
    }
    return p.last_match[0];
}

void CharacterClass2::parseContent() {
    Parser2 p(content);
    negation = p.match('^');
    dash = p.match('-');
    while (!p.is_eof()) {
        Token result;
        result.first = get_char(p);
        if (p.match('-')) {
            result.second = get_char(p);
        } else {
            result.second = result.first;
        }
        tokens.push_back(result);
    }
}

void CharacterClass2::parse(Parser2& p) {
    //~ printf("parsing CharacterClass2\n");
    p.skip_space();
    if (p.match('.')) {
        content = ".";
    } else if (p.match_quoted("[", "]")) {
        content = p.last_match;
        parseContent();
    } else {
        return;
    }
    valid = true;
}


std::string CharacterClass2::to_string() const {
    if (content == ".") {
        return ".";
    } else {
        std::string result;
        if(negation) result += '^';
        if(dash) result += '-';

        for (size_t i = 0; i<tokens.size(); i++) {
            size_t size = tokens[i].second - tokens[i].first;
            switch (size) {
            case 0:
                result += to_char(tokens[i].first);
                break;
            case 1:
                result += to_char(tokens[i].first);
                result += to_char(tokens[i].second);
                break;
            default:
                result += to_char(tokens[i].first);
                result += '-';
                result += to_char(tokens[i].second);
                break;
            }
        }
        return "[" + result + "]";
    }
}

std::string CharacterClass2::dump(std::string indent) const {
    return indent + "CHAR_CLASS " + content;
}

void CharacterClass2::normalize() {
    if (tokens.empty()) {
        return;
    }
    sort(tokens.begin(), tokens.end());
    Tokens init(1, tokens[0]);
    tokens = accumulate(tokens.begin() + 1, tokens.end(), init, [](Tokens& acc, const Token& item){
        Token& prev = acc.back();
        if (prev.second + 1 < item.first) {
            acc.push_back(item);
        } else if (item.second > prev.second) {
            prev.second = item.second;
        }
        return acc;
    });
}
