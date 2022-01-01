#ifndef INCLUDED_C_API_H
#define INCLUDED_C_API_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum AstNodeTypeTag {
    AST_GRAMMAR,
    AST_COMMENT,
    AST_RULE,
    AST_RULE_NAME,
    AST_DIRECTIVE,
    AST_DIRECTIVE_NAME,
    AST_CODE,
    AST_SOURCE,
    AST_ERROR,
    AST_PRIMARY,
    AST_ALTERNATION,
    AST_SEQUENCE,
    AST_PREFIX_OP,
    AST_POSTFIX_OP,
    AST_RULEREF,
    AST_VAR,
    AST_STRING,
    AST_CHARCLASS,
    AST_DOT,
    AST_BACKREF,
    AST_GROUP,
    AST_CAPTURE
} AstNodeType;

typedef void* AstNodeC;

int source_read(void *);

AstNodeC create_ast_leaf(AstNodeType type, const char* text, size_t start);
AstNodeC create_ast_node(AstNodeType type);
void append_child(AstNodeC parent, AstNodeC child);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_C_API_H */
