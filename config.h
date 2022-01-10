#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

#include <map>
#include <string>
#include <vector>

struct Config {
    enum OutputType {
        OT_FORMAT,
        OT_AST,
        OT_DEBUG
    };

    bool optimize;
    bool inplace;
    bool use_double_quotes;
    bool keep_unused_captures;
    bool keep_unused_variables;
    int inline_limit;
    int wrap_limit;
    OutputType output_type;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

private:
    static const Config* instance;
    std::string next;

    typedef int (Config::*MemberFn)();
    std::map<std::string, MemberFn> args;

    void usage(const std::string& error);
    void process_args(const std::vector<std::string>& arguments, const bool config_file);
    void post_process();

    int help();
    int set_optimize();
    int set_inplace();
    int set_format();
    int set_ast();
    int set_debug();
    int set_verbose();
    int set_double_quotes();
    int set_keep_unused_captures();
    int set_keep_unused_variables();
    int set_single_quotes();
    int set_inline_limit();
    int set_wrap_limit();
    int set_input();
    int set_output();
    int load_config();

public:
    static const Config& get();

    Config(int argc, char **argv);
};

#endif /* INCLUDED_CONFIG_H */
