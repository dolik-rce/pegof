#include "ast.h"
#include "peg.h"
#include "source.h"

class Parser {
    Source source;
    peg_context_t *parser;
    AstNode* grammar;

public:
    AstNode* parse();

    Parser(const char* path);
    ~Parser();
};
