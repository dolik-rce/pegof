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
        Io::open(conf.inputs[i], conf.outputs[i]);
        AstNode* grammar = parse();
        if (Config::get<bool>("verbose")) {
            grammar->stats("Loaded grammar containing ");
        }

        switch (conf.output_type) {
        case Config::OT_DEBUG:
            grammar->debug();
            break;
        case Config::OT_AST:
            Config::get<bool>("optimize") && grammar->optimize();
            grammar->print_ast();
            break;
        case Config::OT_FORMAT:
            Config::get<bool>("optimize") && grammar->optimize();
            grammar->format();
        }
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
