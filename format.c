#include "system.h"

#include <ctype.h>

void format__print_terminal(system_t *obj, ast_node_t *node) {
    printf("%.*s", node->len, node->text);
}

void format__print_grammar(system_t *obj, ast_node_t *node) {
    if (node->arity > 0) {
        for (ast_node_t *p = node->child.first; p != NULL; p = p->sibling.next) {
            format__print_node(obj, p);
        }
    }
}

size_t format__count_character(const char* text, range_t range, char ch) {
    size_t count = 0;
    for(size_t i=range.min; i < range.max; ++i) {
        if (text[i] == ch){
            ++count;
        }
    }
    return count;
}

void format__print_string(system_t *obj, ast_node_t *node) {
    // TODO: configurable quotes
    printf("\"");
    for(size_t i=0; i < node->len; ++i) {
        switch (node->text[i]) {
        case '\\':
            const char next = node->text[++i];
            printf("%s%c", next == '\'' ? "" : "\\", next);
            break;
        case '"':
            printf("\\\"");
            break;
        default:
            printf("%c", node->text[i]);
            break;
        }
    }
    printf("\"");
}

void format__print_source(system_t *obj, ast_node_t *node) {
    const char *text = node->text;

    size_t first_non_ws = 0;
    while (isspace(text[first_non_ws])) ++first_non_ws;
    size_t last_non_ws = node->len;
    while (isspace(text[--last_non_ws])) {};
    range_t trimmed_range = range__new(first_non_ws, last_non_ws + 1);

    size_t newlines = format__count_character(obj->source.text.p, trimmed_range, '\n');

    int is_directive = node->parent->type == AST_NODE_TYPE_DIRECTIVE;

    if (newlines == 0) {
        printf(" { %.*s }", (int)(trimmed_range.max - trimmed_range.min), obj->source.text.p + trimmed_range.min);
    } else {
        size_t base_indent = 0; //TODO: check that no line has indent less than base_indent
        while (text[trimmed_range.min - base_indent - 1] != '\n') ++base_indent;
        printf(" {\n");

        printf(is_directive ? "    " : "        ");
        size_t pos = trimmed_range.min;
        while (pos < trimmed_range.max) {
            if(text[pos] == '\n') {
                printf("\n%s", is_directive ? "    " : "        ");
                pos += base_indent;
            } else {
                printf("%c", text[pos]);
            }
            ++pos;
        }
        printf("\n%s}", is_directive ? "" : "    ");
    }
}

void format__print_directive(system_t *obj, ast_node_t *node) {
    ast_node_t* name = node->child.first;
    ast_node_t* content = name->sibling.next;

    printf("%%");
    format__print_node(obj, name);
    if (content->type == AST_NODE_TYPE_STRING) {
        printf(" ");
    }
    format__print_node(obj, content);
    printf("\n\n");
}

void format__print_alternation(system_t *obj, ast_node_t *node) {
    int multiline = node->arity > 1 && node->parent->type == AST_NODE_TYPE_RULE;
    const char *delim = multiline ? "\n    / " : " / ";
    int first = 1;
    for (ast_node_t *p = node->child.first; p != NULL; p = p->sibling.next) {
        if (first == 0) {
            printf("%s", delim);
        }
        format__print_node(obj, p);
        first = 0;
    }
}

void format__print_sequence(system_t *obj, ast_node_t *node) {
    int first = 1;
    for (ast_node_t *p = node->child.first; p != NULL; p = p->sibling.next) {
        if (first == 0) {
            printf(" ");
        }
        format__print_node(obj, p);
        first = 0;
    }
}

void format__print_primary(system_t *obj, ast_node_t *node) {
    for (ast_node_t *p = node->child.first; p != NULL; p = p->sibling.next) {
        format__print_node(obj, p);
    }
}

void format__print_group(system_t *obj, ast_node_t *node, const char* brackets) {
    printf("%c", brackets[0]);
    format__print_node(obj, node->child.first);
    printf("%c", brackets[1]);
}

void format__print_ruleref(system_t *obj, ast_node_t *node) {
    if (node->arity > 0){
        // has variable
        format__print_node(obj, node->child.first);
        printf(":");
    }
    printf("%.*s", node->len, node->text);
}

void format__print_rule(system_t *obj, ast_node_t *node) {
    ast_node_t* name = node->child.first;
    ast_node_t* alternation = name->sibling.next;

    printf("%.*s%s <- ", name->len, name->text, alternation->arity == 1 ? "" : "\n   ");
    format__print_node(obj, alternation);
    printf("\n\n");
}

void *format__print_code(system_t *obj, ast_node_t *node) {
    size_t last_non_ws = node->len;
    while (isspace(node->text[--last_non_ws])) {};
    printf("%%%%\n%.*s\n", last_non_ws + 1, node->text);
}

ast_node_t *format__find_parent_by_type(ast_node_t *node, ast_node_type_t type) {
    ast_node_t *result = node;
    while (result->type != type) {
        if (result->parent == NULL) {
            return NULL;
        }
        result = result->parent;
    }
    return result;
}

void *format__print_comment(system_t *obj, ast_node_t *node) {
    // We have to recognize three different comment positions:
    //   1) top-level comment (parent is GRAMMAR) -> add newline, no indent
    //   2) at the end of sequence in top-lefel alternation -> no newline, no indent
    //   3) anywhere else -> add newline + indent
    const char* suffix = "\n        ";
    if (node->parent->type == AST_NODE_TYPE_GRAMMAR) {
        suffix = "\n";
    } else {
        ast_node_t *parent_alternation = format__find_parent_by_type(node, AST_NODE_TYPE_ALTERNATION);
        if (parent_alternation != NULL
            && parent_alternation->parent->type == AST_NODE_TYPE_RULE
            && node->parent->sibling.next == NULL)
        {
            suffix = "";
        }
    }

    size_t last_non_ws = node->len;
    while (isspace(node->text[--last_non_ws])) {};
    printf("# %.*s%s", last_non_ws + 1, node->text, suffix);
}

void format__print_node(system_t *obj, ast_node_t *node) {
    switch (node->type) {
    case AST_NODE_TYPE_GRAMMAR:         format__print_grammar(obj, node); break;
    case AST_NODE_TYPE_COMMENT:         format__print_comment(obj, node); break;
    case AST_NODE_TYPE_RULE:            format__print_rule(obj, node); break;
    case AST_NODE_TYPE_DIRECTIVE:       format__print_directive(obj, node); break;
    case AST_NODE_TYPE_CODE:            format__print_code(obj, node); break;
    case AST_NODE_TYPE_PRIMARY:         format__print_primary(obj, node); break;
    case AST_NODE_TYPE_ALTERNATION:     format__print_alternation(obj, node); break;
    case AST_NODE_TYPE_SEQUENCE:        format__print_sequence(obj, node); break;
    case AST_NODE_TYPE_RULEREF:         format__print_ruleref(obj, node); break;
    case AST_NODE_TYPE_GROUP:           format__print_group(obj, node, "()"); break;
    case AST_NODE_TYPE_CAPTURE:         format__print_group(obj, node, "<>"); break;
    case AST_NODE_TYPE_SOURCE:          format__print_source(obj, node); break;
    case AST_NODE_TYPE_STRING:          format__print_string(obj, node); break;

    // everything else just print verbatim:
    case AST_NODE_TYPE_RULE_NAME:
    case AST_NODE_TYPE_DIRECTIVE_NAME:
    case AST_NODE_TYPE_VAR:
    case AST_NODE_TYPE_PREFIX_OP:
    case AST_NODE_TYPE_POSTFIX_OP:
    case AST_NODE_TYPE_CHARCLASS:
    case AST_NODE_TYPE_DOT:
    case AST_NODE_TYPE_BACKREF:
        format__print_terminal(obj, node);
        break;
    }
}
