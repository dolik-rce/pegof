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
    OutputType output_type;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

private:
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
    int set_input();
    int set_output();
    int load_config();

public:
    Config(int argc, char **argv);
};

#endif /* INCLUDED_CONFIG_H */
