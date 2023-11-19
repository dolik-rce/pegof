#include "config.h"
#include "utils.h"
#include "log.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string.h>

Config* Config::instance = NULL;
static Config::Option UNKNOWN_OPTION(Config::OptionGroup::OG_BASIC, "", "", nullptr, "");

const std::map<std::string, Optimization> opt_mapping = {
    {"all", O_ALL},
    {"none", O_NONE},
    {"normalize-char-class", O_NORMALIZE_CHAR_CLASS},
    {"inline", O_INLINE},
    {"remove-group", O_REMOVE_GROUP},
    {"single-char-class", O_SINGLE_CHAR_CLASS},
    {"char-class-negation", O_CHAR_CLASS_NEGATION},
    {"double-negation", O_DOUBLE_NEGATION},
    {"double-quantification", O_DOUBLE_QUANTIFICATION},
    {"repeats", O_REPEATS},
    {"concat-strings", O_CONCAT_STRINGS},
    {"unused-variable", O_UNUSED_VARIABLE},
    {"unused-capture", O_UNUSED_CAPTURE},
};

const std::map<Optimization, const char*> opt_descriptions = {
    {O_ALL, {"All optimizations: Shorthand option for combination of all available optimizations."}},
    {O_NONE, {"No optimizations: Shorthand option for no optimizations."}},
    {O_INLINE, {"Rule inlining: Some simple rules can be inlined directly into rules that reference them. Reducing number of rules improves the speed of generated parser."}},
    {O_NORMALIZE_CHAR_CLASS, {"Character class optimization: Normalize character classes to avoid duplicities and use ranges where possible. E.g. `[ABCDEFX0-53-9X]` becomes `[0-9A-FX]`."}},
    {O_REMOVE_GROUP, {"Remove unnecessary groups: Some parenthesis can be safely removed without changeing the meaning of the grammar. E.g.: `A (B C) D` becomes `A B C D` or `X (Y)* Z` becomes `X Y* Z`."}},
    {O_SINGLE_CHAR_CLASS, {"Convert single character classes to strings: The code generated for strings is simpler than that generated for character classes. So we can convert for example `[\\n]` to `\"\\n\"` or `[^ ]` to `!\" \"`."}},
    {O_CHAR_CLASS_NEGATION, {"Simplify negation of character classes: Negation of character classes can be written as negative character class (e.g. `![\\n]` -> `[^\\n]`)."}},
    {O_DOUBLE_NEGATION, {"Removing double negations: Negation of negation can be ignored, because it results in the original term (e.g. `!(!TERM)` -> `TERM`)."}},
    {O_DOUBLE_QUANTIFICATION, {"Removing double quantifications: If a single term is quantified twice, it is always possible to convert this into a single potfix operator with equel meaning (e.g. `(X+)?` -> `X*`)."}},
    {O_REPEATS, {"Removing unnecessary repeats: Joins repeated rules to single quantity. E.g. \"A A*\" -> \"A+\", \"B* B*\" -> \"B*\" etc."}},
    {O_CONCAT_STRINGS, {"String concatenation: Join adjacent string nodes into one. E.g. `\"A\" \"B\"` becomes `\"AB\"`."}},
    {O_UNUSED_VARIABLE, {"Removing unused variables: Variables denoted in grammar (e.g. `e:expression`) which are not used in any source oe error block are discarded."}},
    {O_UNUSED_CAPTURE, {"Removing unused captures: Captures denoted in grammar, which are not used in any source block, error block or referenced (via `$n`) are discarded."}}
};

void Config::usage(const std::string& error_msg) {
    log(0, "PEG Formatter and Optimizer");
    log(0, "");
    log(0, "Usage:");
    log(0, "    pegof [<options>] [--] [<input_file> ...]");
    const char* og_mapping[] = {"Basic", "Input/output", "Formatting", "Optimization"};
    OptionGroup last = OG_OPT;
    for (const Option& opt : args) {
        if (last != opt.group) {
            log(0, "\n%s options:", og_mapping[opt.group]);
            last = opt.group;
        }
        log(0, "    -%s/--%s %s\n        %s", opt.shortName.c_str(), opt.longName.c_str(), opt.param.c_str(), opt.description.c_str());
    }

    log(0, "\nSupported values for --optimize and --exclude options:");
    for (const auto& item : opt_mapping) {
        log(0, "    %s", item.first.c_str());
        log(0, "        %s", replace(opt_descriptions.at(item.second), "`", "").c_str());
    }

    if (!error_msg.empty()) {
        log(0, "");
        error("%s", error_msg.c_str());
    }
}

void Config::usage_markdown() {
    log(0, "### Usage:");
    log(0, "`pegof [<options>] [--] [<input_file> ...]`");
    const char* og_mapping[] = {"Basic", "Input/output", "Formatting", "Optimization"};
    OptionGroup last = OG_OPT;
    for (const Option& opt : args) {
        if (last != opt.group) {
            log(0, "\n### %s options:", og_mapping[opt.group]);
            last = opt.group;
        }
        log(0, "`-%s/--%s %s` %s\n", opt.shortName.c_str(), opt.longName.c_str(), opt.param.c_str(), opt.description.c_str());
    }

    log(0, "### Supported values for --optimize and --exclude options:");
    for (const auto& item : opt_mapping) {
        const char* description = opt_descriptions.at(item.second);
        log(0, "- `%s` %s\n", item.first.c_str(), description);
    }
    log(0, "");
}

int Config::help() {
    usage("");
    exit(0);
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
            }
            std::string next(i+1 == arguments.size() || arguments[i+1][0] == '-' ? "" : arguments[i+1]);
            if (opt.value.type() == typeid(MemberFn)) {
                i += (this->*(std::any_cast<MemberFn>(opt.value)))();
            } else if (opt.value.type() == typeid(MemberFn1)) {
                if (next.empty()) {
                    usage(arg + " requires an argument");
                }
                i += (this->*(std::any_cast<MemberFn1>(opt.value)))(next);
            } else if (opt.value.type() == typeid(OutputType)) {
                output_type = std::any_cast<OutputType>(opt.value);
            } else if (opt.value.type() == typeid(QuoteType)) {
                if (next.empty() || (next != "single" && next != "double")) {
                    usage("Option '" + arg + "' requires argument 'single' or 'double'");
                }
                find_option("quotes").value = next[0] == 's' ? QT_SINGLE : QT_DOUBLE;
                i++;
            } else if (opt.value.type() == typeid(bool)) {
                opt.value = !std::any_cast<bool>(opt.value);
            } else if (opt.value.type() == typeid(int)) {
                if (next.empty()) {
                    usage("Option '" + arg + "' requires an integer argument");
                }
                size_t s = 0;
                opt.value = std::stoi(next, &s);
                if (s != next.size()) {
                    usage("Option '" + arg + "' requires an integer argument, got '" + next + "'");
                }
                i++;
            } else if (opt.value.type() == typeid(double)) {
                if (next.empty()) {
                    usage("Option '" + arg + "' requires an double argument");
                }
                size_t s = 0;
                opt.value = std::stod(next, &s);
                if (s != next.size()) {
                    usage("Option '" + arg + "' requires an double argument, got '" + next + "'");
                }
                i++;
            } else {
                usage("Internal error!");
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
        }
        if (!outputs.empty()) {
            usage("Combining --inplace and --output parameters is not allowed");
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
    }
}

bool Config::get(const Optimization& opt) {
    return instance->optimizations & opt;
}

bool Config::verbose(int level) {
    return instance->verbosity >= level;
}

int Config::parse_optimization_config(const std::string& param) {
    int result = O_NONE;
    std::vector<std::string> parts = split(param);
    for (const std::string& part : parts) {
        auto opt = opt_mapping.find(trim(part));
        if (opt != opt_mapping.end()) {
            result |= opt->second;
        } else {
            usage("Unrecognized optimization '" + part + "'!");
        }
    }
    return result;
}

int Config::parse_optimize(const std::string& param) {
    optimizations |= parse_optimization_config(param);
    return 1;
}

int Config::parse_exclude(const std::string& param) {
    optimizations &= ~parse_optimization_config(param);
    return 1;
}

int Config::inc_verbosity() {
    verbosity++;
    return 0;
}

Config::Config(int argc, char **argv) : output_type(OT_FORMAT), optimizations(O_NONE), verbosity(0) {
    if (instance) {
        error("Internal error, Config instance already exists!");
    }
    instance = this;

    args = {
        Option(OG_BASIC, "h", "help", &Config::help, "Show help (this text)"),
        Option(OG_BASIC, "c", "conf", &Config::load_config, "Use given configuration file", "FILE"),
        Option(OG_BASIC, "v", "verbose", &Config::inc_verbosity, "Verbose logging to stderr (repeat for even more verbose output)"),
        Option(OG_BASIC, "d", "debug", false, "Output very verbose debug info, implies max verbosity"),
        Option(OG_BASIC, "S", "skip-validation", false, "Skip result validation (useful only for debugging purposes)"),
        Option(OG_IO, "f", "format", OT_FORMAT, "Output formatted grammar (default)"),
        Option(OG_IO, "a", "ast", OT_AST, "Output abstract syntax tree representation"),
        Option(OG_IO, "p", "packcc", OT_PACKCC, "Output source files as if the grammar was passed to packcc"),
        Option(OG_IO, "I", "inplace", false, "Modify the input files (only when formatting)"),
        Option(OG_IO, "i", "input", &Config::set_input, "Path to file with PEG grammar, multiple paths can be given\n        Value \"-\" can be used to specify standard input\n        Mainly useful for config file\n        If no file or --input is given, read standard input.", "FILE"),
        Option(OG_IO, "o", "output", &Config::set_output, "Output to file (should be repeated if there is more inputs)\n        Value \"-\" can be used to specify standard output\n        Must not be used together with --inplace.", "FILE"),
        Option(OG_FORMAT, "q", "quotes", QT_DOUBLE, "Switch between double and single quoted strings (defaults to double)", "single/double"),
        Option(OG_FORMAT, "w", "wrap-limit", 1, "Wrap alternations with more than N sequences (default 1)", "N"),
        Option(OG_OPT, "O", "optimize", &Config::parse_optimize, "Comma separated list of optimizations to apply", "OPT[,...]"),
        Option(OG_OPT, "X", "exclude", &Config::parse_exclude, "Comma separated list of optimizations that should not be applied", "OPT[,...]"),
        Option(OG_OPT, "l", "inline-limit", 0.2, "Minimum inlining score needed for rule to be inlined.\n        Number between 0.0 (inline everything) and 1.0 (most conservative), default is 0.2,\n        only applied when inlining is enabled", "N"),
    };
    std::vector<std::string> arguments(argv + 1, argv + argc);
    if (argc == 2 && strcmp(argv[1], "--usage-markdown") == 0) {
        usage_markdown();
        exit(0);
    }
    process_args(arguments, false);
    post_process();
}
