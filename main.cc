#include "source.h"
#include "ast.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

enum command_t {
    CMD_NONE = 0,
    CMD_AST = 1,
    CMD_OPTIMIZE = 2,
    CMD_DEBUG = 4
};


const char *process_args(int argc, char **argv, command_t& cmd) {
    if (argc == 2) {
        cmd = CMD_NONE;
        return argv[1];
    } else if (argc == 3) {
        if (strcmp("--ast", argv[1]) == 0) {
            cmd = CMD_AST;
        } else if (strcmp("--format", argv[1]) == 0) {
            cmd = CMD_NONE;
        } else if (strcmp("--optimize", argv[1]) == 0) {
            cmd = CMD_OPTIMIZE;
        } else if (strcmp("--optimize-ast", argv[1]) == 0) {
            cmd = command_t(CMD_OPTIMIZE|CMD_AST);
        } else if (strcmp("--debug", argv[1]) == 0) {
            cmd = CMD_DEBUG;
        } else {
            fprintf(stderr, "ERROR: Unknown option '%s'!\n", argv[1]);
            return NULL;
        }
        return argv[2];
    } else {
        fprintf(stderr, "ERROR: Wrong number of arguments!\n");
        return NULL; /* usage error */
    }
}

class Parser {
    parser_context_t *parser;
    AstNode* grammar;
public:
    AstNode* parse() {
        const int res = parser_parse(parser, (AstNodeC*)&grammar);
        if (res != 0) {
            exit(10);
            fprintf(stderr, "FATAL: Could not parse the grammar.\n");
        }
        if (!grammar) {
            exit(11);
            fprintf(stderr, "FATAL: Could not parse the grammar.\n");
        }
        return grammar;
    }

    Parser(Source& source) : parser(parser_create(&source)), grammar(NULL) {}
    ~Parser() {
        parser_destroy(parser);
        if (grammar) {
            delete grammar;
        }
    }
};


int main(int argc, char **argv) {
    command_t cmd;
    const char *path = process_args(argc, argv, cmd);
    if (path == NULL) {
        return 1;
    }

    Source source = Source(path);
    Parser parser = Parser(source);
    AstNode* grammar = parser.parse();

    if (cmd & CMD_DEBUG) {
        printf("### Original AST:\n");
        grammar->print_ast();
        printf("### Original formatted:\n");
        grammar->format();
        printf("### Optmizations made: %d\n", grammar->optimize());
        printf("### Optimized AST:\n");
        grammar->print_ast();
        printf("### Optimized formatted:\n");
        grammar->format();
        return 0;
    }

    if (cmd & CMD_OPTIMIZE) {
        grammar->optimize();
    }
    if (cmd & CMD_AST) {
        grammar->print_ast();
    } else {
        grammar->format();
    }

    return 0;
}
