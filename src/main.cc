#include "parser.h"
#include "ast/grammar.h"
#include "checker.h"
#include "optimizer.h"
#include "config.h"
#include "utils.h"
#include "log.h"
#include "version.h"

Grammar parse(const std::string& input, const Checker& checker) {
    std::string content = read_file(input);
    if (content.empty()) {
        error("Failed to read grammar '%s'", input.c_str());
    }

    log(1, "Validating input grammar ...");
    checker.validate(input, content);

    log(1, "Parsing grammar ...");
    Parser peg(content);
    Grammar g(peg, input);
    if (!g) {
        error("Failed to parse grammar!");
    }
    return g;
}

void process(const Config::OutputType& output_type, const std::string& input, const std::string& output, const Checker& checker) {
    log(1, "Processing file %s, storing output to %s ...",
        input.empty() ? "stdin" : input.c_str(),
        output.empty() ? "stdout" : output.c_str());

    Grammar g = parse(input, checker);
    g.update_parents();
    Stats in_stats = checker.stats(g);

    if (Config::get(O_ALL)) {
        log(1, "Optimizing grammar ...");
        Optimizer opt(g);
        g = opt.optimize();
        g.update_parents();
    }

    std::string result;
    if (output_type != Config::OT_AST) {
        log(1, "Validating formatted grammar ...");
        result = g.to_string();
        checker.validate_string("formatted.peg", result);
    }

    if (Config::get(O_ALL) && (Config::verbose(1) || !Config::get<std::string>("benchmark").empty())) {
        log(1, "Computing stats ...");
        Stats out_stats = checker.stats(g);
        log(0, "%s", out_stats.compare(in_stats).c_str());
    }

    switch (output_type) {
    case Config::OT_FORMAT:
        log(1, "Writing formatted output ...");
        write_file(output, result);
        break;
    case Config::OT_AST:
        log(1, "Writing AST ...");
        write_file(output, g.dump() + "\n");
        break;
    case Config::OT_PACKCC:
        if (output.empty()) error("Option -p/--packcc requires output to file, use -o/--output!");
        log(1, "Processing with PackCC ...");
        checker.packcc(result, output);
        log(1, "Parser was generated in %s.{h,c}", output.c_str());
        break;
    }

}

int main(int argc, char **argv) {
    try {
        Config conf(argc, argv);
        debug("Pegof version: %s", pegof_version.c_str());
        debug("PackCC version: %s", pcc_version.c_str());
        Checker checker;

        for (int i = 0; i < conf.inputs.size(); i++) {
            const std::string& input = conf.inputs[i];
            const std::string& output = conf.outputs[i];
            checker.set_input_file(input);
            process(conf.output_type, input, output, checker);
        }
        return 0;
    } catch (int e) {
        return e;
    }
}
