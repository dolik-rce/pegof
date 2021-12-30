#include "parser.h"

AstNode* Parser::parse() {
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

Parser::Parser(const char* path) : source(path), parser(peg_create(&source)), grammar(NULL) {}

Parser::~Parser() {
    peg_destroy(parser);
    if (grammar) {
        delete grammar;
    }
}
