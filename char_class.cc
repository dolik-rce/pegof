#include "char_class.h"

#include <algorithm>
#include <iomanip>
#include <numeric>

int CharacterClass::get_char(const std::string& s, size_t& pos) {
    if(s[pos++] == '\\') {
        switch (s[pos++]) {
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
            int c = std::stoi(std::string(s.c_str() + pos, 4), nullptr, 16);
            pos += 4;
            return c;
        }
    }
    return s[pos-1];
}

std::string CharacterClass::to_unicode(const int c) {
    std::stringstream ss;
    ss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << c;
    return ss.str();
}

std::string CharacterClass::to_char(int c) {
    if (c > 127) {
        return to_unicode(c);
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

CharacterClass::Token CharacterClass::get_range(const std::string& s, size_t& pos) {
    Token result;
    result.first = get_char(s, pos);
    if (s.size() > pos && s[pos] == '-') {
        pos++;
        result.second = get_char(s, pos);
    } else {
        result.second = result.first;
    }
    return result;
}

CharacterClass::Tokens CharacterClass::tokenize(const std::string& input) {
    Tokens tokens;
    size_t pos = 0;
    while (pos < input.size()) {
        Token r;
        tokens.push_back(get_range(input, pos));
    }

    return tokens;
}

CharacterClass::Tokens CharacterClass::join(const CharacterClass::Tokens& tokens) {
    Tokens init(1, tokens[0]);
    return accumulate(tokens.begin() + 1, tokens.end(), init, [](Tokens& acc, const Token& item){
        Token& prev = acc.back();
        if (prev.second + 1 < item.first) {
            acc.push_back(item);
        } else if (item.second > prev.second) {
            prev.second = item.second;
        }
        return acc;
    });
}

std::string CharacterClass::normalize(const std::string& str) {
    bool negation = str[0] == '^';
    bool dash = str[negation] == '-';
    std::string input(str, negation + dash);

    Tokens tokens = tokenize(input);
    sort(tokens.begin(), tokens.end());
    Tokens normalized = join(tokens);

    std::string r;
    if(negation) r.push_back('^');
    if(dash) r.push_back('-');

    for (size_t i=0; i<normalized.size(); i++) {
        size_t size = normalized[i].second - normalized[i].first;
        switch (size) {
        case 0:
            r += to_char(normalized[i].first);
            break;
        case 1:
            r += to_char(normalized[i].first);
            r += to_char(normalized[i].second);
            break;
        default:
            r += to_char(normalized[i].first);
            r.push_back('-');
            r += to_char(normalized[i].second);
            break;
        }
    }
    return r;
}
