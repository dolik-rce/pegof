#pragma once
#include <string>
#include <regex>

extern "C" {
#include "capi.h"
}

class Parser {
    std::string input;
    unsigned long pos;

public:
    friend void convert_parser(void* parser, char** input, size_t* len, unsigned long* pos);
    friend void update_parser(void* parser, unsigned long pos);

    struct State {
        Parser* p;
        unsigned long saved_pos;

        State(Parser* p);
        bool rollback();
        bool commit();
        bool commit(int start, int end);
        bool commit(const std::string& result);
    };

    std::smatch last_re_match;
    std::string last_match;

    Parser(std::string input);

    State save_point();

    bool is_eof();
    void skip_space();
    void skip_rest_of_line(bool continuable);

    bool match(char c);
    bool match(const std::string& str, bool space = true);
    bool match_re(const std::string& r, bool space = true);

    bool match_comment();

    bool match_block_comment();
    bool match_line_comment();

    bool match_macro();
    bool match_string();
    bool match_code();

    bool match_number();
    bool match_identifier();

    bool peek(const char c);
    bool peek_re(const std::string& r, bool space = true);
};
