#include "config.h"

#include <algorithm>

extern bool debug_mode;

void Config::usage(const std::string& error) {
    printf("PEG Formatter and Optimizer\n");
    printf("\n");
    printf("Usage:\n");
    printf("    pegof [-h|--help|--usage]");
    printf("    pegof [-O|--optimize] [-f|-a|--format|--ast|-d|--debug] \\\n");
    printf("          [-i|--inplace|[-o|--output <output_file>]*] \\\n");
    printf("          [--] [<input_file> ...]\n");
    printf("\n");
    printf("Arguments:\n");
    printf("    -h, --help      Print this text\n");
    printf("    -O, --optimize  Try to optimize the input grammar\n");
    printf("    -f, --format    Output formatted grammar (default)\n");
    printf("    -a, --ast       Output abstract syntax tree representation\n");
    printf("    -d, --debug     Output debug info (includes AST and formatted output)\n");
    printf("    -i, --inplace   Modify the input files (only when formatting)\n");
    printf("    -o, --output    Output to file (should be repeated if there is more inputs)\n");
    printf("                    Value \"-\" can be used to specify standard output\n");
    printf("                    Must not be used together with --inplace\n");
    printf("    <input_file>    Path to file with PEG grammar, multiple paths can be given\n");
    printf("                    If no file is given, read standard input\n");
    printf("                    Value \"-\" can also be used to read from standard input.\n");
    if (!error.empty()) {
        printf("\nERROR: %s\n", error.c_str());
    }
}

int Config::help() {
    usage("");
    exit(0);
    return 0;
}

int Config::set_optimize() {
    optimize = true;
    return 0;
}

int Config::set_inplace() {
    inplace = true;
    return 0;
}

int Config::set_format() {
    output_type = OT_FORMAT;
    return 0;
}

int Config::set_ast() {
    output_type = OT_AST;
    return 0;
}

int Config::set_debug() {
    debug_mode = true;
    output_type = OT_DEBUG;
    return 0;
}

int Config::set_output() {
    if (next.empty()) {
        usage("-o/--output requires an argument");
        exit(1);
    }
    outputs.push_back(next);
    return 1;
}

void Config::process_args(int argc, char **argv) {
    bool all_inputs = false;
    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--") {
            all_inputs = true;
            continue;
        }
        if (all_inputs) {
            inputs.push_back(arg);
            continue;
        }
        if (args.count(arg) == 0) {
            if (arg[0] == '-') {
                usage("Unknown argument: '" + arg + "'\n");
                exit(1);
            } else {
                inputs.push_back(arg);
                continue;
            }
        }
        next = i+1 == argc ? "" : argv[i+1];
        i += (this->*args[arg])();
    }
}

void Config::post_process() {
    if (inputs.empty()) {
        // if no file was given, read stdin
        inputs.push_back("");
    }

    if (inplace) {
        if (output_type != OT_FORMAT) {
            usage("Inplace editing is only allowed when formatting code");
            exit(1);
        }
        if (!outputs.empty()) {
            usage("Combining --inplace and --output parameters is not allowed");
            exit(1);
        }
        outputs = inputs;
    } else if (outputs.empty()) {
        // if no outputs were given, assume stdout
        outputs.resize(inputs.size(), "");
    }

    std::replace(inputs.begin(), inputs.end(), std::string("-"), std::string());
    std::replace(outputs.begin(), outputs.end(), std::string("-"), std::string());

    if (inputs.size() != outputs.size()) {
        usage("Number of inputs does not match number of outputs");
        exit(1);
    }
}

Config::Config(int argc, char **argv) : optimize(false), inplace(false), output_type(OT_FORMAT) {
    args["-h"] = &Config::help;
    args["--help"] = &Config::help;
    args["--usage"] = &Config::help;
    args["-O"] = &Config::set_optimize;
    args["--optimize"] = &Config::set_optimize;
    args["-f"] = &Config::set_format;
    args["--format"] = &Config::set_format;
    args["-a"] = &Config::set_ast;
    args["--ast"] = &Config::set_ast;
    args["-d"] = &Config::set_debug;
    args["--debug"] = &Config::set_debug;
    args["-i"] = &Config::set_inplace;
    args["--inplace"] = &Config::set_inplace;
    args["-o"] = &Config::set_output;
    args["--output"] = &Config::set_output;

    process_args(argc, argv);
    post_process();
}
