#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

#include <any>
#include <map>
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
    O_ALL = 4095
};

struct Config {
    enum OutputType {
        OT_FORMAT,
        OT_AST,
        OT_PACKCC
    };

    enum QuoteType {
        QT_DOUBLE,
        QT_SINGLE
    };

    OutputType output_type;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

    typedef int (Config::*MemberFn)();
    typedef int (Config::*MemberFn1)(const std::string&);

    enum OptionGroup {
        OG_BASIC,
        OG_IO,
        OG_FORMAT,
        OG_OPT
    };

    struct Option {
        OptionGroup group;
        std::string shortName;
        std::string longName;
        std::any value;
        std::string description;
        std::string param;

        template<typename T>
        Option(
            OptionGroup group,
            const std::string& shortName,
            const std::string& longName,
            T defaultValue,
            const std::string& description,
            const std::string& param = ""
        ) : group(group), shortName(shortName), longName(longName), value(defaultValue), description(description), param(param)
        {}
    };

private:
    static Config* instance;

    std::vector<Option> args;
    int optimizations;
    int verbosity;

    void usage(const std::string& error);
    void usage_markdown();
    void process_args(const std::vector<std::string>& arguments, const bool config_file);
    void post_process();

    int help();
    int set_input(const std::string& next);
    int set_output(const std::string& next);
    int load_config(const std::string& next);
    int parse_optimization_config(const std::string& param);
    int parse_optimize(const std::string& param);
    int parse_exclude(const std::string& param);
    int inc_verbosity();

    Option& find_option(const std::string& optionName);

public:
    template<typename T>
    static const T get(std::string optionName) {
        return std::any_cast<T>(instance->find_option(optionName).value);
    }

    static bool get(const Optimization& opt);
    static bool verbose(int level);

    Config(int argc, char **argv);
};

#endif /* INCLUDED_CONFIG_H */
