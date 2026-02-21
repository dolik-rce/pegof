#pragma once
#include <any>
#include <map>
#include <set>
#include <string>
#include <vector>

enum Optimization {
    O_NONE = 0,
    O_NORMALIZE_CHAR_CLASS = 1,
    O_INLINE = 2,
    O_REMOVE_GROUP = 4,
    O_SINGLE_CHAR_CLASS = 8,
    //~ O_CHAR_CLASS_NEGATION = 16,
    O_DOUBLE_NEGATION = 32,
    O_DOUBLE_QUANTIFICATION = 64,
    O_REPEATS = 128,
    O_CONCAT_STRINGS = 256,
    O_CONCAT_CHAR_CLASSES = 512,
    O_UNUSED_VARIABLE = 1024,
    O_UNUSED_CAPTURE = 2048,
    O_EMPTY_ACTION = 4096,
    O_SAME_RULES = 8192,
    O_REPEATED_SEQUENCE = 16384,
    O_ALL = 32767
};

enum HeaderMode { HM_UNSET = -1, HM_NEVER = 0, HM_AUTO = 1, HM_ALWAYS = 2 };

struct Config {
    enum OutputType { OT_UNSET, OT_FORMAT, OT_AST, OT_GRAPH, OT_PACKCC };

    enum QuoteType { QT_UNSET, QT_DOUBLE, QT_SINGLE };

    OutputType output_type;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<std::string> import_dirs;

    typedef int (Config::*MemberFn)();
    typedef int (Config::*MemberFn1)(const std::string&);
    typedef int (Config::*MemberFnOpt1)(const std::string&, int optional);

    enum OptionGroup { OG_BASIC, OG_IO, OG_FORMAT, OG_OPT };

    struct Option {
        OptionGroup group;
        std::string shortName;
        std::string longName;
        std::any value;
        std::any unsetValue;
        std::any defaultValue;
        std::string description;
        std::string param;

        template<typename T>
        Option(
            OptionGroup group, const std::string& shortName, const std::string& longName, T unsetValue, T defaultValue,
            const std::string& description, const std::string& param = ""
        ):
            group(group),
            shortName(shortName),
            longName(longName),
            value(unsetValue),
            unsetValue(unsetValue),
            defaultValue(defaultValue),
            description(description),
            param(param) {}

        template<typename T>
        Option(
            OptionGroup group, const std::string& shortName, const std::string& longName, T setterFunction,
            const std::string& description, const std::string& param
        ):
            group(group),
            shortName(shortName),
            longName(longName),
            value(setterFunction),
            description(description),
            param(param) {}
    };

private:
    static Config* instance;

    std::vector<Option> args;
    int optimizations;
    int verbosity;
    std::string indent;
    HeaderMode header;
    std::set<char> packcc_options;

    void usage(const std::string& error);
    void usage_markdown();
    void process_args(const std::vector<std::string>& arguments, const bool config_file);
    void post_process();

    int help();
    int version();
    int set_input(const std::string& next);
    int set_output(const std::string& next);
    int set_import(const std::string& next);
    int set_indent(const std::string& next);
    int set_packcc_options(const std::string& next);
    int load_config(const std::string& next);
    int parse_optimization_config(const std::string& param);
    int parse_optimize(const std::string& param);
    int parse_header(const std::string& param);
    int parse_exclude(const std::string& param);
    int set_verbosity(const std::string& next, int);

    Option& find_option(const std::string& optionName);

    template<typename T> T check_conflict(const std::string& option, T previous_value, T unset, T new_value) {
        if (previous_value != unset && previous_value != new_value) {
            usage("Conflicting value for " + option + ".");
        }
        return new_value;
    }

    template<typename T> void set_default(const char* optionName) {
        Option& opt = find_option(optionName);
        if (std::any_cast<T>(opt.value) == std::any_cast<T>(opt.unsetValue)) {
            opt.value = opt.defaultValue;
        }
    }

public:
    template<typename T> static const T get(std::string optionName) {
        return std::any_cast<T>(instance->find_option(optionName).value);
    }

    static bool get(const Optimization& opt);
    static bool get(const HeaderMode& headerMode);
    static std::string get_opt_name(const Optimization& opt);
    static const Config& get();
    static const std::string& get_indent();
    static const std::set<char>& get_packcc_options();
    static std::vector<std::string> get_all_imports_dirs(const std::string& input_file);
    static bool verbose(int level);

    Config(int argc, char** argv);
};
