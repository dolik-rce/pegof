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

    typedef void (Config::*MemberFn)();
    std::map<std::string, MemberFn> args;

    void process_args(int argc, char **argv);

    void usage(const std::string& error);
    void help();
    void set_optimize();
    void set_inplace();
    void set_format();
    void set_ast();
    void set_debug();
    void set_output();

public:
    Config(int argc, char **argv);
};

#endif /* INCLUDED_CONFIG_H */
