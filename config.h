#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

#include <any>
#include <map>
#include <string>
#include <vector>

struct Config {
    enum OutputType {
        OT_FORMAT,
        OT_AST,
        OT_DEBUG
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

    void usage(const std::string& error);
    void process_args(const std::vector<std::string>& arguments, const bool config_file);
    void post_process();

    int help();
    int set_input(const std::string& next);
    int set_output(const std::string& next);
    int load_config(const std::string& next);

    Option& find_option(const std::string& optionName);

public:
    template<typename T>
    static const T get(std::string optionName) {
        return std::any_cast<T>(instance->find_option(optionName).value);
    }

    Config(int argc, char **argv);
};

#endif /* INCLUDED_CONFIG_H */
