#ifndef INCLUDED_C_API_H
#define INCLUDED_C_API_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ast_node_type_tag {
    AST_NODE_TYPE_GRAMMAR,
    AST_NODE_TYPE_COMMENT,
    AST_NODE_TYPE_RULE,
    AST_NODE_TYPE_RULE_NAME,
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

typedef void* AstNodeC;
typedef void* SourceC;

int source_read(SourceC source);

AstNodeC create_ast_leaf(SourceC obj, ast_node_type_t type, const char* text, size_t start);
AstNodeC create_ast_node(ast_node_type_t type);
void append_child(AstNodeC parent, AstNodeC child);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_C_API_H */
