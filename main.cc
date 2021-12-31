#include "parser.h"
#include "config.h"

#include <stdio.h>
#include <string.h>

bool debug_mode = false;

int main(int argc, char **argv) {
    Config conf(argc, argv);
    for (int i = 0; i < conf.inputs.size(); i++) {

        Parser parser = Parser(conf.inputs[i].c_str());
        AstNode* grammar = parser.parse();

        switch (conf.output_type) {
        case Config::OT_DEBUG:
            debug_mode = true;
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

    }
    return 0;
}
