#include "parser.h"

Parser2::State::State(Parser2* p) : p(p), saved_pos(p->pos) {};

bool Parser2::State::rollback() {
    if (p->pos != saved_pos) {
        printf("rollback %lu-%lu: %s\n", saved_pos, p->pos, p->input.substr(saved_pos, p->pos - saved_pos).c_str());
    }
    p->pos = saved_pos;
    return false;
};

bool Parser2::State::commit() {
    p->last_match = p->input.substr(saved_pos, p->pos - saved_pos);
    printf("matched %lu-%lu: %s\n", saved_pos, p->pos, p->last_match.c_str());
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
        if (!match_comment()) break;
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

void Parser2::skip_rest_of_line(bool continuable) {
    while (!is_eof()) {
        if (continuable) match("\\\n");
        if (match('\n')) break;
        pos++;
    }
}

bool Parser2::match_comment() {
    State s(this);
    if (!match('#')) {
        return s.rollback();
    }
    while (input[pos] != '\n' && !is_eof()) pos++;
    return s.commit();
}

bool Parser2::match_block_comment() {
    return match_quoted("/*", "*/");
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
    if (match(left)) {
        while (!match(right)) {
            match('\\');
            pos++;
        }
        return s.commit();
    }
    return s.rollback();
}

bool Parser2::match_code() {
    State s(this);
    skip_space();
    if (!match('{')) {
        return s.rollback();
    }
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
    return s.commit();
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


/*
Grammar* Parser::parse() {
    const int res = peg_parse(parser, (AstNodeC*)&grammar);
    if (res != 0) {
        exit(10);
        fprintf(stderr, "FATAL: Could not parse the grammar.\n");
    }
    if (!grammar) {
        exit(11);
        fprintf(stderr, "FATAL: Could not parse the grammar.\n");
    }
    return grammar;
}

int Parser::parse_all() {
    for (int i = 0; i < conf.inputs.size(); i++) {
        Io::open(conf.inputs[i], conf.outputs[i]);
        Grammar* grammar = parse();
        printf("DBG: %s\n", grammar->to_string().c_str());

        //~ if (Config::get<bool>("verbose")) {
            //~ grammar->stats("Loaded grammar containing ");
        //~ }

        //~ switch (conf.output_type) {
        //~ case Config::OT_DEBUG:
            //~ grammar->debug();
            //~ break;
        //~ case Config::OT_AST:
            //~ Config::get<bool>("optimize") && grammar->optimize();
            //~ grammar->print_ast();
            //~ break;
        //~ case Config::OT_FORMAT:
            //~ Config::get<bool>("optimize") && grammar->optimize();
            //~ grammar->format();
        //~ }
        if (grammar) {
            delete grammar;
        }
        Io:: close();
    }
    return 0;
}

Parser::Parser(const Config& conf) : conf(conf), parser(peg_create(NULL)), grammar(NULL) {}

Parser::~Parser() {
    peg_destroy(parser);
}
*/
