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

int Parser::parse_all() {
    for (int i = 0; i < conf.inputs.size(); i++) {
        Io::open(conf.inputs[i]);
        AstNode* grammar = parse();

        switch (conf.output_type) {
        case Config::OT_DEBUG:
            fprintf(stderr, "### Original AST:\n");
            grammar->print_ast();
            fprintf(stderr, "### Original formatted:\n");
            grammar->format();
            fprintf(stderr, "### Optimizing:\n");
            grammar->optimize();
            fprintf(stderr, "### Optimized AST:\n");
            grammar->print_ast();
            fprintf(stderr, "### Optimized formatted:\n");
            grammar->format();
            break;
        case Config::OT_AST:
            conf.optimize && grammar->optimize();
            grammar->print_ast();
            break;
        case Config::OT_FORMAT:
            conf.optimize && grammar->optimize();
            grammar->format();
        }
        if (grammar) {
            delete grammar;
        }
    }
    return 0;
}

Parser::Parser(const Config& conf) : conf(conf), parser(peg_create(NULL)), grammar(NULL) {}

Parser::~Parser() {
    peg_destroy(parser);
}
