#include "parser.h"

Parser2::State::State(Parser2* p) : p(p), saved_pos(p->pos) {};

bool Parser2::State::rollback() {
    //~ if (p->pos != saved_pos) {
        //~ printf("rollback %lu-%lu: %s\n", saved_pos, p->pos, p->input.substr(saved_pos, p->pos - saved_pos).c_str());
    //~ }
    p->pos = saved_pos;
    return false;
};

bool Parser2::State::commit() {
    p->last_match = p->input.substr(saved_pos, p->pos - saved_pos);
    //~ printf("matched %lu-%lu: %s\n", saved_pos, p->pos, p->last_match.c_str());
    return true;
};

bool Parser2::State::commit(int start, int end) {
    p->last_match = p->input.substr(start, end - start);
    //~ printf("matched %lu-%lu: %s\n", saved_pos, p->pos, p->last_match.c_str());
    return true;
};

bool Parser2::State::commit(const std::string& result) {
    p->last_match = result;
    //~ printf("matched %lu-%lu: %s\n", saved_pos, p->pos, p->last_match.c_str());
    return true;
};

Parser2::Parser2(std::string input) : input(input), pos(0) {}

bool Parser2::is_eof() {
    return pos == input.size();
}

void Parser2::skip_space() {
    while (true) {
        if (isspace(input[pos])) {
            pos++;
            continue;
        }
        break;
        //~ if (!(include_comments && match_comment())) break;
    }
}

bool Parser2::match(char c) {
    if (input[pos] == c) {
        pos++;
        last_match = std::string(1, c);
        return true;
    }
    return false;
}

bool Parser2::match(const std::string& str) {
    State s(this);
    skip_space();
    if (input.compare(pos, str.size(), str) == 0) {
        pos += str.size();
        return s.commit();
    }
    return s.rollback();
}

bool Parser2::match_re(const std::string& r) {
    State s(this);
    skip_space();
    std::smatch m;
    if (std::regex_search(input.cbegin() + pos, input.cend(), m, std::regex(r))) {
        if (m.position(0) == 0) {
            pos += m.length(0);
            last_re_match = m;
            return s.commit();
        }
    }
    return s.rollback();
}

bool Parser2::match_any() {
    if (is_eof()) return false;
    last_match = std::string(1, input[pos]);
    pos++;
    return true;
}

void Parser2::skip_rest_of_line(bool continuable) {
    while (!is_eof()) {
        if (continuable) match("\\\n");
        if (match('\n')) break;
        pos++;
    }
}

bool Parser2::match_comment() {
    std::string result = "";
    while (match_re("[ \t]*#[ \t]*([^\\n]*\\n)")) {
        result += last_re_match.str(1);
    }
    if (!result.empty() && result.back() == '\n') {
        result.resize(result.size() - 1);
    }
    last_match = result;
    return !result.empty();
}

bool Parser2::match_block_comment() {
    State s(this);
    if (!match("/*")) return s.rollback();
    unsigned long start = pos;
    for (; !match("*/"); pos++) {}
    return s.commit(start, pos - 2);
}

bool Parser2::match_line_comment() {
    if (match("//")) {
        skip_rest_of_line(false);
        return true;
    }
    return false;
}

bool Parser2::match_macro() {
    if (match('#')) {
        skip_rest_of_line(true);
        return true;
    }
    return false;
}

bool Parser2::match_quoted(const char *left, const char *right) {
    State s(this);
    skip_space();
    std::string result;
    if (match(left)) {
        int start = pos;
        for (; !match(right); pos++) {
            if (match('\\')) {
                switch (input[pos]) {
                case '\x00': result += '\0'; break;
                case 'f': result += '\f'; break;
                case 't': result += '\t'; break;
                case 'v': result += '\v'; break;
                case 'r': result += '\r'; break;
                case 'n': result += '\n'; break;
                case 'a': result += '\a'; break;
                case 'b': result += '\b'; break;
                default: result += input[pos];
                }
            } else {
                result += input[pos];
            }
        }
        return s.commit(result);
    }
    return s.rollback();
}

bool Parser2::match_code() {
    State s(this);
    skip_space();
    if (!match('{')) {
        return s.rollback();
    }
    int start = pos;
    int level = 1;
    while (true) {
        if (is_eof()) {
            printf("Premature EOF in code block starting @%ul\n", s.saved_pos);
            return s.rollback();
        }
        if (
            match_macro() ||
            match_line_comment() ||
            match_block_comment() ||
            match_quoted("\"", "\"") ||
            match_quoted("\'", "\'")
        ) continue;
        if (match('{')) {
            level++;
        } else if (match('}')) {
            level--;
            if (level == 0) break;
        } else {
            pos++;
        }
    }
    return s.commit(start, pos - 1);
}

bool Parser2::match_number() {
    return match_re("[0-9]+");
}

bool Parser2::match_identifier() {
    return match_re("[_a-zA-Z][_a-zA-Z0-9]*");
}

bool Parser2::peek(const char c) {
    State s(this);
    bool result = match(c);
    s.rollback();
    return result;
}

bool Parser2::peek(const std::string& str) {
    State s(this);
    bool result = match(str);
    s.rollback();
    return result;
}

bool Parser2::peek_re(const std::string& r) {
    State s(this);
    bool result = match_re(r);
    s.rollback();
    return result;
}

char Parser2::current() const {
    return input[pos];
}
