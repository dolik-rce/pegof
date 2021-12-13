/*
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "system.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

typedef enum command_tag {
    CMD_AST,
    CMD_FORMAT,
    CMD_OPTIMIZE
} command_t;


const char *process_args(int argc, char **argv, command_t* cmd) {
    if (argc == 2) {
        *cmd = CMD_AST;
        return argv[1];
    } else if (argc == 3) {
        if (strcmp("--ast", argv[1]) == 0) {
            *cmd = CMD_AST;
        } else if (strcmp("--format", argv[1]) == 0) {
            *cmd = CMD_FORMAT;
        } else if (strcmp("--optimize", argv[1]) == 0) {
            *cmd = CMD_OPTIMIZE;
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
    const char *path = process_args(argc, argv, &cmd);
    if (path == NULL) {
        return 1;
    }
    int ret = 0;
    system_t system;
    parser_context_t *parser = NULL;
    if (setjmp(system.jmp) == 0) {
        system__initialize(&system);
        system__open_source_file(&system, path);
        parser = parser_create(&system);
        ast_node_t *ast;
        const int b = parser_parse(parser, &ast);
        if (b) {
            ret = 10; /* internal error */
            fprintf(stderr, "FATAL: Internal error\n");
                /* <-- input text remaining due to incompleteness of the grammar */
        }
        else {
            switch (cmd) {
            case CMD_AST:
                system__dump_ast(&system, ast);
                break;
            case CMD_FORMAT:
                format__print_node(&system, ast);
                break;
            case CMD_OPTIMIZE:
                fprintf(stderr, "Not implemented yet, sorry...\n");
                break;
            default: break;
            }
        }
    }
    else {
        ret = 2; /* error during parsing */
    }
    parser_destroy(parser);
    system__finalize(&system); /* all system resources are freed */
    return ret;
}
