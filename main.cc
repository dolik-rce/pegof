#include "parser.h"
#include "grammar.h"
#include "checker.h"
#include "optimizer.h"
#include "config.h"
#include "utils.h"

Grammar parse(const std::string& input, const std::string& output, const Checker& checker) {
    std::string content = read_file(input);
    if (content.empty()) {
        //~ printf("ERROR: Failed to load grammar from %s!\n", input.empty() ? "stdin" : input.c_str());
        exit(1);
    }

    if (!checker.validate(input, content)) {
        exit(1);
    }

    Parser peg(content);
    Grammar g(peg);
    if (!g) {
        printf("ERROR: Failed to parse grammar!\n");
        exit(1);
    }
    return g;
}

void process(const std::string& input, const std::string& output, const Checker& checker) {
    Grammar g = parse(input, output, checker);
    g.update_parents();

    if (Config::get<bool>("optimize")) {
        Optimizer opt(g);
        g = opt.optimize();
        g.update_parents();
    }

    std::string result = g.to_string();
    if (!checker.validate_string("formatted.peg", result)) {
        printf("ERROR: Formatted grammar is invalid!\n");
        exit(1);
    }
    checker.stats();
    write_file(output, result);
}

int main(int argc, char **argv) {
    Config conf(argc, argv);
    Checker checker;

    for (int i = 0; i < conf.inputs.size(); i++) {
        const std::string& input = conf.inputs[i];
        const std::string& output = conf.outputs[i];

        //~ printf("Processing '%s' -> '%s'\n", input.c_str(), output.c_str());
        switch (conf.output_type) {
        case Config::OT_FORMAT:
            process(input, output, checker);
            break;
        case Config::OT_AST:
            printf("%s\n", parse(input, output, checker).dump().c_str());
            break;
        case Config::OT_DEBUG:
            printf("ERROR: Debug mode is not implemeted yet");
            exit(1);
            break;
        }
    }
    return 0;
}
