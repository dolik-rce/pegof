#include "parser.h"

#include <stdio.h>
#include <string.h>

enum command_t {
    CMD_NONE = 0,
    CMD_AST = 1,
    CMD_OPTIMIZE = 2,
    CMD_DEBUG = 4
};

bool debug_mode = false;

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

int main(int argc, char **argv) {
    command_t cmd;
    const char *path = process_args(argc, argv, cmd);
    if (path == NULL) {
        return 1;
    }

    Parser parser = Parser(path);
    AstNode* grammar = parser.parse();

    if (cmd & CMD_DEBUG) {
        debug_mode = true;
        fprintf(stderr, "### Original AST:\n");
        grammar->print_ast();
        fprintf(stderr, "### Original formatted:\n");
        grammar->format();
        fprintf(stderr, "### Optimizing:\n");
        grammar->optimize();
        fprintf(stderr, "### Optimized AST:\n");
        grammar->print_ast();
        fprintf(stderr, "### Optimized formatted:\n");
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
