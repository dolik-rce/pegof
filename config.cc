#include "config.h"
#include "io.h"

#include <algorithm>
#include <fstream>
#include <iterator>

Config* Config::instance = NULL;
static Config::Option UNKNOWN_OPTION(Config::OptionGroup::OG_BASIC, "", "", nullptr, "");

void Config::usage(const std::string& error) {
    printf("PEG Formatter and Optimizer\n");
    printf("\n");
    printf("Usage:\n");
    printf("    pegof [<options>] [--] [<input_file> ...]\n");
    const char* og_mapping[] = {"Basic", "Input/output", "Formatting", "Optimization"};
    OptionGroup last = OG_OPT;
    for (const Option& opt : args) {
        if (last != opt.group) {
            printf("\n%s options:\n", og_mapping[opt.group]);
            last = opt.group;
        }
        printf("    -%s/--%s %s\n        %s\n", opt.shortName.c_str(), opt.longName.c_str(), opt.param.c_str(), opt.description.c_str());
    }

    if (!error.empty()) {
        printf("\nERROR: %s\n", error.c_str());
    }
}

int Config::help() {
    usage("");
    exit(0);
    return 0;
}

int Config::set_input(const std::string& next) {
    inputs.push_back(next);
    return 1;
}

int Config::set_output(const std::string& next) {
    outputs.push_back(next);
    return 1;
}

int Config::load_config(const std::string& next) {
    std::vector<std::string> arguments;
    std::ifstream file(next);
    std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), std::back_inserter(arguments));

    process_args(arguments, true);
    return 1;
}

Config::Option& Config::find_option(const std::string& optionName) {
    for (Option& option : instance->args) {
        if (option.longName == optionName || option.shortName == optionName) {
            return option;
        }
    }
    return UNKNOWN_OPTION;
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
            Option& opt = find_option(arg_name);
            if (&opt == &UNKNOWN_OPTION) {
                usage("Unknown option: '" + arg + "'\n");
                exit(1);
            }
            std::string next(i+1 == arguments.size() || arguments[i+1][0] == '-' ? "" : arguments[i+1]);
            if (opt.value.type() == typeid(MemberFn)) {
                i += (this->*(std::any_cast<MemberFn>(opt.value)))();
            } else if (opt.value.type() == typeid(MemberFn1)) {
                if (next.empty()) {
                    usage(arg + " requires an argument");
                    exit(1);
                }
                i += (this->*(std::any_cast<MemberFn1>(opt.value)))(next);
            } else if (opt.value.type() == typeid(OutputType)) {
                output_type = std::any_cast<OutputType>(opt.value);
            } else if (opt.value.type() == typeid(QuoteType)) {
                if (next.empty() || (next != "single" && next != "double")) {
                    usage("Option '" + arg + "' requires argument 'single' or 'double'");
                    exit(1);
                }
                find_option("quotes").value = next[0] == 's' ? QT_SINGLE : QT_DOUBLE;
                i++;
            } else if (opt.value.type() == typeid(bool)) {
                opt.value = !std::any_cast<bool>(opt.value);
            } else if (opt.value.type() == typeid(int)) {
                if (next.empty()) {
                    usage("Option '" + arg + "' requires an integer argument");
                    exit(1);
                }
                size_t s = 0;
                opt.value = std::stoi(next, &s);
                if (s != next.size()) {
                    usage("Option '" + arg + "' requires an integer argument, got '" + next + "'");
                    exit(1);
                }
                i++;
            } else {
                usage("Internal error!");
                exit(3);
            }
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

    if (Config::get<bool>("inplace")) {
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

Config::Config(int argc, char **argv) : output_type(OT_FORMAT) {
    if (instance) {
        fprintf(stderr, "Internal error, Config instance already exists!\n");
    }
    instance = this;

    args = {
        Option(OG_BASIC, "h", "help", &Config::help, "Show help (this text)"),
        Option(OG_BASIC, "c", "conf", &Config::load_config, "Use given configuration file", "FILE"),
        Option(OG_BASIC, "v", "verbose", false, "Verbose logging to stderr"),
        Option(OG_IO, "f", "format", OT_FORMAT, "Output formatted grammar (default)"),
        Option(OG_IO, "a", "ast", OT_AST, "Output abstract syntax tree representation"),
        Option(OG_IO, "d", "debug", OT_DEBUG, "Output debug info (includes AST and formatted output)"),
        Option(OG_IO, "I", "inplace", false, "Modify the input files (only when formatting)"),
        Option(OG_IO, "i", "input", &Config::set_input, "Path to file with PEG grammar, multiple paths can be given\n        Value \"-\" can be used to specify standard input\n        Mainly useful for config file\n        If no file or --input is given, read standard input.", "FILE"),
        Option(OG_IO, "o", "output", &Config::set_output, "Output to file (should be repeated if there is more inputs)\n        Value \"-\" can be used to specify standard output\n        Must not be used together with --inplace.", "FILE"),
        Option(OG_FORMAT, "q", "quotes", QT_DOUBLE, "Switch between double and single quoted strings (defaults to double)", "single/double"),
        Option(OG_FORMAT, "w", "wrap-limit", 1, "Wrap alternations with more than N sequences (default 1)", "N"),
        Option(OG_OPT, "O", "optimize", false, "Apply optimizations"),
        Option(OG_OPT, "C", "keep-captures", false, "Do not discard unused captures"),
        Option(OG_OPT, "V", "keep-variables", false, "Do not discard unused variables"),
        Option(OG_OPT, "r", "keep-repeats", false, "Do not optimize repeated tokens"),
        Option(OG_OPT, "n", "no-concat", false, "Do not concatenate adjacent string"),
        Option(OG_OPT, "N", "no-char-class", false, "Do not optimize character classes"),
        Option(OG_OPT, "l", "inline-limit", 10, "Maximum number of references rule can have and still\n        be inlined (default 10)", "N"),
    };
    std::vector<std::string> arguments(argv + 1, argv + argc);
    process_args(arguments, false);
    post_process();
}
