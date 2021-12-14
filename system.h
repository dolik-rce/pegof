#ifndef INCLUDED_SYSTEM_H
#define INCLUDED_SYSTEM_H

#include "utility.h"

#include <stdio.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct system_tag system_t;
typedef struct ast_node_tag ast_node_t;

typedef enum ast_node_type_tag {
    AST_NODE_TYPE_GRAMMAR,
    AST_NODE_TYPE_COMMENT,
    AST_NODE_TYPE_RULE,
    AST_NODE_TYPE_DIRECTIVE,
    AST_NODE_TYPE_DIRECTIVE_NAME,
    AST_NODE_TYPE_CODE,
    AST_NODE_TYPE_SOURCE,
    AST_NODE_TYPE_PRIMARY,
    AST_NODE_TYPE_ALTERNATION,
    AST_NODE_TYPE_SEQUENCE,
    AST_NODE_TYPE_PREFIX_OP,
    AST_NODE_TYPE_POSTFIX_OP,
    AST_NODE_TYPE_RULEREF,
    AST_NODE_TYPE_VAR,
    AST_NODE_TYPE_STRING,
    AST_NODE_TYPE_CHARCLASS,
    AST_NODE_TYPE_DOT,
    AST_NODE_TYPE_BACKREF,
    AST_NODE_TYPE_GROUP,
    AST_NODE_TYPE_CAPTURE
} ast_node_type_t;

struct ast_node_tag {
    ast_node_type_t type; /* the AST node type */
    range_t range; /* the byte range in the source text */
    size_t arity; /* the number of the child AST nodes */
    ast_node_t *parent; /* the parent AST node */
    struct ast_node_sibling_tag {
        ast_node_t *prev; /* the previous sibling AST node */
        ast_node_t *next; /* the next sibling AST node */
    } sibling;
    struct ast_node_child_tag {
        ast_node_t *first; /* the first child AST node */
        ast_node_t *last; /* the last child AST node */
    } child;
    system_t *system; /* the system that manages this AST node */
    struct ast_node_managed_tag {
        ast_node_t *prev; /* the previous AST node managed by the same system */
        ast_node_t *next; /* the next AST node managed by the same system */
    } managed;
};

struct system_tag {
    struct system_input_tag {
        const char *path; /* the source file path */
        FILE *file; /* the source file pointer */
        char_array_t text; /* the source text */
        size_t_array_t line; /* the byte positions of the line head in the source text */
    } source;
    struct system_managed_tag {
        ast_node_t *first; /* the first managed AST node */
        ast_node_t *last; /* the last managed AST node */
    } managed;
    jmp_buf jmp;
};

void system__initialize(system_t *obj);
void system__finalize(system_t *obj);

void *system__allocate_memory(system_t *obj, size_t size);
void *system__reallocate_memory(system_t *obj, void *ptr, size_t size);
void system__deallocate_memory(system_t *obj, void *ptr);

void system__open_source_file(system_t *obj, const char *path); /* the standard input if path == NULL */
void system__close_source_file(system_t *obj);
int system__read_source_file(system_t *obj);

ast_node_t *system__create_ast_node_terminal(system_t *obj, ast_node_type_t type, range_t range);
ast_node_t *system__create_ast_node_unary(system_t *obj, ast_node_type_t type, range_t range, ast_node_t *node1);
ast_node_t *system__create_ast_node_binary(system_t *obj, ast_node_type_t type, range_t range, ast_node_t *node1, ast_node_t *node2);
ast_node_t *system__create_ast_node_ternary(system_t *obj, ast_node_type_t type, range_t range, ast_node_t *node1, ast_node_t *node2, ast_node_t *node3);
ast_node_t *system__create_ast_node_variadic(system_t *obj, ast_node_type_t type, range_t range);

void system__destroy_all_ast_nodes(system_t *obj);

void system__dump_ast(system_t *obj, ast_node_t *root);

void ast_node__prepend_child(ast_node_t *obj, ast_node_t *node);
void ast_node__append_child(ast_node_t *obj, ast_node_t *node);
void ast_node__destroy(ast_node_t *obj);

void format__print_node(system_t *obj, ast_node_t *node);

#ifdef __cplusplus
}
#endif

#endif /* !INCLUDED_SYSTEM_H */
