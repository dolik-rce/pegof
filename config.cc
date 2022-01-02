#include "config.h"
#include "io.h"

#include <algorithm>
#include <fstream>
#include <iterator>

void Config::usage(const std::string& error) {
    printf("PEG Formatter and Optimizer\n");
    printf("\n");
    printf("Usage:\n");
    printf("    pegof [-h|--help|--usage]");
    printf("    pegof [-c|--conf <config_file>] [-O|--optimize] \\\n");
    printf("          [-f|-a|--format|--ast|-d|--debug] \\\n");
    printf("          [-i|--inplace|[-o|--output <output_file>]*] \\\n");
    printf("          [--] [<input_file> ...]\n");
    printf("\n");
    printf("Arguments:\n");
    printf("    -h, --help      Print this text\n");
    printf("    -c, --conf      Use given configuration file\n");
    printf("    -O, --optimize  Try to optimize the input grammar\n");
    printf("    -f, --format    Output formatted grammar (default)\n");
    printf("    -a, --ast       Output abstract syntax tree representation\n");
    printf("    -d, --debug     Output debug info (includes AST and formatted output)\n");
    printf("    -I, --inplace   Modify the input files (only when formatting)\n");
    printf("    -i, --input     Path to file with PEG grammar, multiple paths can be given.\n");
    printf("                    Value \"-\" can be used to specify standard input\n");
    printf("                    Mainly useful for config file\n");
    printf("    -o, --output    Output to file (should be repeated if there is more inputs)\n");
    printf("                    Value \"-\" can be used to specify standard output\n");
    printf("                    Must not be used together with --inplace\n");
    printf("    <input_file>    Any non-arguments will be treated if passed to --input\n");
    printf("                    If no file or --input is given, read standard input\n");
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
    Io::set_debug_mode(true);
    output_type = OT_DEBUG;
    return 0;
}

int Config::set_input() {
    if (next.empty()) {
        usage("-i/--input requires an argument");
        exit(1);
    }
    inputs.push_back(next);
    return 1;
}

int Config::set_output() {
    if (next.empty()) {
        usage("-o/--output requires an argument");
        exit(1);
    }
    outputs.push_back(next);
    return 1;
}

int Config::load_config() {
    if (next.empty()) {
        usage("-c/--conf requires an argument");
        exit(1);
    }

    std::vector<std::string> arguments;
    std::ifstream file(next);
    std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), std::back_inserter(arguments));

    process_args(arguments, true);
    return 1;
}

void Config::process_args(const std::vector<std::string>& arguments, const bool config_file) {
    bool all_inputs = false;
    for (int i = 0; i < arguments.size(); i++) {
        const std::string& arg = arguments[i];
        if (arg == "--") {
            all_inputs = true;
            continue;
        }
        if (all_inputs) {
            inputs.push_back(arg);
            continue;
        }
        if ((arg[0] == '-' && arg.size() > 1) || config_file) {
            // strip leading dashes from arguments (unless we read arguments from config file)
            std::string arg_name = config_file ? arg : arg.substr(arg[1] == '-' ? 2 : 1);
            if (args.count(arg_name) == 0) {
                usage("Unknown " + std::string(config_file ? "config file option" : "argument") + ": '" + arg + "'\n");
                exit(1);
            }
            next = i+1 == arguments.size() ? "" : arguments[i+1];
            i += (this->*args[arg_name])();
            continue;
        }
        // treat anything not starting with dash as input
        inputs.push_back(arg);
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
    args["h"] = &Config::help;
    args["help"] = &Config::help;
    args["usage"] = &Config::help;
    args["c"] = &Config::load_config;
    args["conf"] = &Config::load_config;
    args["O"] = &Config::set_optimize;
    args["optimize"] = &Config::set_optimize;
    args["f"] = &Config::set_format;
    args["format"] = &Config::set_format;
    args["a"] = &Config::set_ast;
    args["ast"] = &Config::set_ast;
    args["d"] = &Config::set_debug;
    args["debug"] = &Config::set_debug;
    args["I"] = &Config::set_inplace;
    args["inplace"] = &Config::set_inplace;
    args["i"] = &Config::set_input;
    args["input"] = &Config::set_input;
    args["o"] = &Config::set_output;
    args["output"] = &Config::set_output;

    std::vector<std::string> arguments(argv + 1, argv + argc);
    process_args(arguments, false);
    post_process();
}
