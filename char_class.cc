#include "char_class.h"

#include <algorithm>
#include <iomanip>
#include <numeric>

int CharacterClass::get_char(const std::string& s, size_t& pos) const {
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

CharacterClass::Token CharacterClass::get_range(const std::string& s, size_t& pos) const {
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

std::string CharacterClass::to_unicode(const int c) const {
    std::stringstream ss;
    ss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << c;
    return ss.str();
}

std::string CharacterClass::to_char(int c) const {
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

int CharacterClass::size() const {
    int sum = dash;
    for (size_t i = 0; i<tokens.size(); i++) {
        sum += 1 + (tokens[i].second - tokens[i].first);
    }
    return sum;
}

std::string CharacterClass::to_string() const {
    std::string r;
    if(negation) r.push_back('^');
    if(dash) r.push_back('-');

    for (size_t i = 0; i<tokens.size(); i++) {
        size_t size = tokens[i].second - tokens[i].first;
        switch (size) {
        case 0:
            r += to_char(tokens[i].first);
            break;
        case 1:
            r += to_char(tokens[i].first);
            r += to_char(tokens[i].second);
            break;
        default:
            r += to_char(tokens[i].first);
            r.push_back('-');
            r += to_char(tokens[i].second);
            break;
        }
    }
    return r;
}

CharacterClass& CharacterClass::normalize() {
    if (tokens.empty()) {
        return *this;
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
    return *this;
}

CharacterClass::CharacterClass(const std::string str) {
    negation = str[0] == '^';
    dash = str[negation] == '-';
    size_t pos = negation + dash;
    while (pos < str.size()) {
        Token r;
        tokens.push_back(get_range(str, pos));
    }
}
