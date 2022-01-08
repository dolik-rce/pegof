#include "config.h"
#include "io.h"

#include <algorithm>
#include <fstream>
#include <iterator>

const Config* Config::instance = NULL;

void Config::usage(const std::string& error) {
    printf("PEG Formatter and Optimizer\n");
    printf("\n");
    printf("Usage:\n");
    printf("    pegof [-h|--help|--usage]\n");
    printf("    pegof [<options>] [--] [<input_file> ...]\n");
    printf("\n");
    printf("Basic options:\n");
    printf("    -h, --help      Print this text\n");
    printf("    -c, --conf      Use given configuration file\n");
    printf("    -v, --verbose   Verbose logging to stderr\n");
    printf("\n");
    printf("Input/output options:\n");
    printf("    -f, --format    Output formatted grammar (default)\n");
    printf("    -a, --ast       Output abstract syntax tree representation\n");
    printf("    -d, --debug     Output debug info (includes AST and formatted output)\n");
    printf("    -I, --inplace   Modify the input files (only when formatting)\n");
    printf("    -i, --input     Path to file with PEG grammar, multiple paths can be given\n");
    printf("                    Value \"-\" can be used to specify standard input\n");
    printf("                    Mainly useful for config file\n");
    printf("    -o, --output    Output to file (should be repeated if there is more inputs)\n");
    printf("                    Value \"-\" can be used to specify standard output\n");
    printf("                    Must not be used together with --inplace\n");
    printf("    <input_file>    Any non-arguments will be treated if passed to --input\n");
    printf("                    If no file or --input is given, read standard input\n");
    printf("\n");
    printf("Formating options:\n");
    printf("    --double-quotes Use double quoted strings (default)\n");
    printf("    --single-quotes Use single quoted strings\n");
    printf("    --wrap-limit N  Wrap alternations with more than N sequences (default 1)\n");
    printf("\n");
    printf("Optimization options:\n");
    printf("    -O, --optimize      Apply optimizations\n");
    printf("    --inline-limit N    Maximum number of references rule can have\n");
    printf("                        and still be inlined (default 10)\n");

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
    output_type = OT_DEBUG;
    return 0;
}

int Config::set_verbose() {
    Io::set_verbose(true);
    return 0;
}

int Config::set_single_quotes() {
    use_double_quotes = false;
    return 0;
}

int Config::set_double_quotes() {
    use_double_quotes = true;
    return 0;
}

int Config::set_inline_limit() {
    if (next.empty()) {
        usage("--inline-limit requires an argument");
        exit(1);
    }
    inline_limit = atoi(next.c_str());
    return 1;
}

int Config::set_wrap_limit() {
    if (next.empty()) {
        usage("--wrap-limit requires an argument");
        exit(1);
    }
    wrap_limit = atoi(next.c_str());
    return 1;
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
            next = i+1 == arguments.size() || arguments[i+1][0] == '-' ? "" : arguments[i+1];
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

const Config& Config::get() {
    return *instance;
}

Config::Config(int argc, char **argv) :
    optimize(false), inplace(false), use_double_quotes(true), inline_limit(10), wrap_limit(1), output_type(OT_FORMAT)
{
    if (instance) {
        fprintf(stderr, "Internal error, Config instance already exists!\n");
    }

    args["h"] = &Config::help;
    args["help"] = &Config::help;
    args["usage"] = &Config::help;
    args["c"] = &Config::load_config;
    args["conf"] = &Config::load_config;
    args["v"] = &Config::set_verbose;
    args["verbose"] = &Config::set_verbose;
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
    args["double-quotes"] = &Config::set_double_quotes;
    args["single-quotes"] = &Config::set_single_quotes;
    args["inline-limit"] = &Config::set_inline_limit;
    args["wrap-limit"] = &Config::set_wrap_limit;
    args["input"] = &Config::set_input;
    args["o"] = &Config::set_output;
    args["output"] = &Config::set_output;

    std::vector<std::string> arguments(argv + 1, argv + argc);
    process_args(arguments, false);
    post_process();
    instance = this;
}
