#include "config.h"

extern bool debug_mode;

void Config::usage(const std::string& error) {
    printf("PEG Formatter and Optimizer\n");
    printf("\n");
    printf("Usage:\n");
    printf("    pegof [-h|--help|--usage]");
    printf("    pegof [-O|--optimize] [-f|-a|--format|--ast|-d|--debug] \\\n");
    printf("          [[-o|--output <output_file>]*|-i|--inplace] \\\n");
    printf("          [--] <input_file> ...\n");
    printf("\n");
    printf("Arguments:\n");
    printf("    -h, --help      Print this text\n");
    printf("    -O, --optimize  Try to optimize the input grammar\n");
    printf("    -f, --format    Output formatted grammar (default)\n");
    printf("    -a, --ast       Output abstract syntax tree representation\n");
    printf("    -d, --debug     Output debug info (includes AST and formatted output)\n");
    printf("    -o, --output    Output to file (should be repeated if there is more inputs)\n");
    printf("    -i, --inplace   Modify the input files (only when formatting)\n");
    if (!error.empty()) {
        printf("\nERROR: %s\n", error.c_str());
    }
}

void Config::help() {
    usage("");
    exit(0);
}

void Config::set_optimize() {
    optimize = true;
}

void Config::set_inplace() {
    inplace = true;
}

void Config::set_format() {
    output_type = OT_FORMAT;
}

void Config::set_ast() {
    output_type = OT_AST;
}

void Config::set_debug() {
    debug_mode = true;
    output_type = OT_DEBUG;
}

void Config::set_output() {
    if (next.empty()) {
        usage("-o/--output requires an argument");
        exit(1);
    }
    outputs.push_back(next);
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
        (this->*args[arg])();
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
    args["-i"] = &Config::set_debug;
    args["--inplace"] = &Config::set_inplace;
    args["-o"] = &Config::set_output;
    args["--output"] = &Config::set_output;

    process_args(argc, argv);
}
