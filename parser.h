#pragma once

#include <string>
#include <regex>

//~ #include "ast.h"
//~ #include "peg.h"
//~ #include "io.h"
//~ #include "config.h"

class Parser2 {
    std::string input;
    unsigned long pos;

    struct State {
        unsigned long saved_pos;
        Parser2* p;

        State(Parser2* p);
        bool rollback();
        bool commit();
    };

public:
    std::smatch last_re_match;
    std::string last_match;

    Parser2(std::string input);

    bool is_eof();
    void skip_space();
    void skip_rest_of_line(bool continuable);

    bool match(char c);
    bool match(const std::string& str);
    bool match_re(const std::string& r);

    bool match_comment();

    bool match_block_comment();
    bool match_line_comment();

    bool match_macro();
    bool match_quoted(const char *left, const char *right);
    bool match_code();

    bool match_number();
    bool match_identifier();

    bool peek(const char c);
    bool peek(const std::string& str);
    bool peek_re(const std::string& r);
};

//~ class Parser {
    //~ const Config& conf;
    //~ peg_context_t *parser;
    //~ Grammar* grammar;

    //~ Grammar* parse();
//~ public:
    //~ int parse_all();

    //~ Parser(const Config& conf);
    //~ ~Parser();
//~ };
