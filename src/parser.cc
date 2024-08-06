#include "parser.h"
#include "log.h"
#include "utils.h"
#include "packcc_wrapper.h"

Parser::State::State(Parser* p) : p(p), saved_pos(p->pos) {}

bool Parser::State::rollback() {
    if (p->pos != saved_pos) debug("Rollback %lu-%lu: %s", saved_pos, p->pos, to_c_string(p->input.substr(saved_pos, p->pos - saved_pos)).c_str());
    p->pos = saved_pos;
    return false;
}

bool Parser::State::commit() {
    p->last_match = p->input.substr(saved_pos, p->pos - saved_pos);
    debug("Matched %lu-%lu: %s", saved_pos, p->pos, to_c_string(p->last_match).c_str());
    return true;
}

bool Parser::State::commit(int start, int end) {
    p->last_match = p->input.substr(start, end - start);
    debug("Matched %lu-%lu: %s", saved_pos, p->pos, to_c_string(p->last_match).c_str());
    return true;
}

bool Parser::State::commit(const std::string& result) {
    p->last_match = result;
    debug("Matched %lu-%lu: %s", saved_pos, p->pos, to_c_string(p->last_match).c_str());
    return true;
}

Parser::Parser(std::string input) : input(input), pos(0) {}

Parser::State Parser::save_point() {
    return State(this);
}

bool Parser::is_eof() {
    return pos == input.size();
}

void Parser::skip_space() {
    int start = pos;
    while (true) {
        if (isspace(input[pos])) {
            pos++;
            continue;
        }
        break;
    }
    if (pos != start) debug("Skipped whitespace: %d-%d", start, pos);
}

bool Parser::match(char c) {
    if (input[pos] == c) {
        pos++;
        last_match = std::string(1, c);
        return true;
    }
    return false;
}

bool Parser::match(const std::string& str, bool space) {
    State s(this);
    if (space) skip_space();
    if (input.compare(pos, str.size(), str) == 0) {
        pos += str.size();
        return s.commit();
    }
    return s.rollback();
}

bool Parser::match_re(const std::string& r, bool space) {
    State s(this);
    if (space) skip_space();
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

bool Parser::match_any() {
    if (is_eof()) return false;
    last_match = std::string(1, input[pos]);
    pos++;
    return true;
}

void Parser::skip_rest_of_line(bool continuable) {
    while (!is_eof()) {
        if (continuable) match("\\\n");
        if (match('\n')) break;
        pos++;
    }
}

bool Parser::match_comment() {
    //~ printf("DBG: matching comment @%d: %s\n", pos, input.substr(pos).c_str());
    std::string result;
    bool matched = false;
    if (match_re("[ \t]*#([^\\n]*\\n)", false)) {
        result = last_re_match.str(1);
        matched = true;
    }
    if (!result.empty() && result.back() == '\n') {
        result.resize(result.size() - 1);
    }
    last_match = result;
    //~ printf("DBG: result: '%s' matched: %d\n", result.c_str(), matched);
    return matched;
}

bool Parser::match_block_comment() {
    State s(this);
    if (!match("/*")) return s.rollback();
    unsigned long start = pos;
    for (; !match("*/"); pos++) {}
    return s.commit(start, pos - 2);
}

bool Parser::match_line_comment() {
    if (match("//")) {
        skip_rest_of_line(false);
        return true;
    }
    return false;
}

bool Parser::match_macro() {
    if (match('#')) {
        skip_rest_of_line(true);
        return true;
    }
    return false;
}

bool Parser::match_string() {
    State s(this);
    skip_space();
    if (peek('"') || peek('\'')) {
        std::string result;
        if (pcc_match_quoted(this, &result)) {
            return s.commit(result);
        }
    }
    return s.rollback();
}

bool Parser::match_code() {
    State s(this);
    skip_space();
    if (!match('{')) {
        return s.rollback();
    }
    int start = pos;
    int level = 1;
    while (true) {
        if (is_eof()) {
            error("Premature EOF in code block starting @%ul", s.saved_pos);
            return s.rollback();
        }
        if (
            match_macro() ||
            match_line_comment() ||
            match_block_comment() ||
            match_string()
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

bool Parser::match_number() {
    return match_re("[0-9]+");
}

bool Parser::match_identifier() {
    return match_re("[_a-zA-Z][_a-zA-Z0-9]*");
}

bool Parser::peek(const char c) {
    State s(this);
    bool result = match(c);
    s.rollback();
    return result;
}

bool Parser::peek(const std::string& str) {
    State s(this);
    bool result = match(str);
    s.rollback();
    return result;
}

bool Parser::peek_re(const std::string& r, bool space) {
    State s(this);
    bool result = match_re(r, space);
    s.rollback();
    return result;
}

char Parser::current() const {
    return input[pos];
}
