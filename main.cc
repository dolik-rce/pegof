#include "parser.h"
#include "grammar.h"
#include "checker.h"
#include "config.h"
#include "utils.h"


int main(int argc, char **argv) {

    Config conf(argc, argv);
    Checker checker;

    for (int i = 0; i < conf.inputs.size(); i++) {
        if (!checker.validate_file(conf.inputs[i])) {
            exit(1);
        }

        Parser2 peg(read_file(conf.inputs[i]));
        Grammar g(peg);
        if (!g) {
            printf("ERROR: Failed to parse grammar!\n");
            exit(1);
        }

        std::string output;
        switch (conf.output_type) {
        case Config::OT_FORMAT:
            output = g.to_string();
            if (!checker.validate_string(output)) {
                printf("ERROR: Formatted grammar is invalid!\n");
                exit(1);
            }
            printf("%s\n", output.c_str());
            break;
        case Config::OT_AST:
            printf("%s\n", g.dump().c_str());
            break;
        case Config::OT_DEBUG:
            printf("ERROR: Debug mode is not implemeted yet");
            exit(1);
            break;
        }
    }
    return 0;
}
