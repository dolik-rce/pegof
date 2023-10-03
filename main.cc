#include "parser.h"
#include "ast/grammar.h"
#include "checker.h"
#include "optimizer.h"
#include "config.h"
#include "utils.h"
#include "log.h"

Grammar parse(const std::string& input, const std::string& output, const Checker& checker) {
    std::string content = read_file(input);
    if (content.empty()) {
        error("Failed to read grammar '%s'", input.c_str());
    }

    checker.validate(input, content);

    Parser peg(content);
    Grammar g(peg);
    if (!g) {
        error("Failed to parse grammar!");
    }
    return g;
}

void process(const std::string& input, const std::string& output, const Checker& checker) {
    Grammar g = parse(input, output, checker);
    g.update_parents();
    Stats in_stats = checker.stats(g);

    if (Config::get(O_ALL)) {
        Optimizer opt(g);
        g = opt.optimize();
        g.update_parents();
    }

    std::string result = g.to_string();
    checker.validate_string("formatted.peg", result);

    if (Config::get(O_ALL)) {
        Stats out_stats = checker.stats(g);
        log(1, "%s", out_stats.compare(in_stats).c_str());
    }

    write_file(output, result);
}

int main(int argc, char **argv) {
    Config conf(argc, argv);
    Checker checker;

    for (int i = 0; i < conf.inputs.size(); i++) {
        const std::string& input = conf.inputs[i];
        const std::string& output = conf.outputs[i];

        log(1, "Processing file %s, storing output to %s...", input.empty() ? "stdin" : input.c_str(), output.empty() ? "stdout" : output.c_str());
        switch (conf.output_type) {
        case Config::OT_FORMAT:
            process(input, output, checker);
            break;
        case Config::OT_AST:
            printf("%s\n", parse(input, output, checker).dump().c_str());
            break;
        }
    }
    return 0;
}
