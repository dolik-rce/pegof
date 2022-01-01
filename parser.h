#ifndef INCLUDED_PARSER_H
#define INCLUDED_PARSER_H

#include "ast.h"
#include "peg.h"
#include "source.h"
#include "config.h"

class Parser {
    const Config& conf;
    Source source;
    peg_context_t *parser;
    AstNode* grammar;

    AstNode* parse();
public:
    int parse_all();

    Parser(const Config& conf);
    ~Parser();
};

#endif /* INCLUDED_PARSER_H */
