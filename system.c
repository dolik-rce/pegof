#ifdef _MSC_VER
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS /* suppress the warning of fopen() use */
#endif /* _MSC_VER */

#include "system.h"

#include <stdlib.h>

static void append_text_character_(system_t *obj, char c) {
    const size_t n = obj->source.text.n;
    if (!char_array__resize(&(obj->source.text), n + 1)) {
        fprintf(stderr, "FATAL: Out of memory\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    obj->source.text.p[n] = (char)c;
}

static void append_line_head_(system_t *obj, size_t h) {
    const size_t n = obj->source.line.n;
    if (!size_t_array__resize(&(obj->source.line), n + 1)) {
        fprintf(stderr, "FATAL: Out of memory\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    obj->source.line.p[n] = h;
}

static size_t count_characters_(const char *buf, size_t start, size_t end) {
    /* UTF-8 multibyte character support but without checking UTF-8 validity */
    size_t n = 0, i = start;
    while (i < end) {
        const int c = (int)(unsigned char)buf[i];
        if (c == 0) break;
        n++;
        i += (c < 0x80) ? 1 : ((c & 0xe0) == 0xc0) ? 2 : ((c & 0xf0) == 0xe0) ? 3 : ((c & 0xf8) == 0xf0) ? 4 : /* invalid code */ 1;
    }
    return n;
}

static void compute_line_and_column_(system_t *obj, size_t pos, size_t *line, size_t *column) {
    size_t i;
    for (i = 1; i < obj->source.line.n; i++) {
        if (pos < obj->source.line.p[i]) break;
    }
    if (line) *line = i;
    if (column) *column = count_characters_(obj->source.text.p, obj->source.line.p[i - 1], pos) + 1;
}

void system__initialize(system_t *obj) {
    obj->source.path = NULL;
    obj->source.file = NULL;
    char_array__initialize(&(obj->source.text));
    size_t_array__initialize(&(obj->source.line));
    obj->managed.first = NULL;
    obj->managed.last = NULL;
    append_line_head_(obj, 0);
}

void system__finalize(system_t *obj) {
    if (obj->source.file != NULL && obj->source.file != stdin) fclose(obj->source.file);
    char_array__finalize(&(obj->source.text));
    size_t_array__finalize(&(obj->source.line));
    system__destroy_all_ast_nodes(obj);
}

void *system__allocate_memory(system_t *obj, size_t size) {
    void *const p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "FATAL: Out of memory\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    return p;
}

void *system__reallocate_memory(system_t *obj, void *ptr, size_t size) {
    void *const p = realloc(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "FATAL: Out of memory\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    return p;
}

void system__deallocate_memory(system_t *obj, void *ptr) {
    free(ptr);
}

void system__open_source_file(system_t *obj, const char *path) {
    if (path != NULL) {
        FILE *const f = fopen(path, "rt");
        if (f == NULL) {
            fprintf(stderr, "FATAL: Cannot open file '%s'\n", path);
            longjmp(obj->jmp, 1); /* never returns */
        }
        obj->source.path = path;
        obj->source.file = f;
    }
    else {
        obj->source.path = NULL;
        obj->source.file = stdin;
    }
}

void system__close_source_file(system_t *obj) {
    if (obj->source.file != NULL && obj->source.file != stdin) {
        if (fclose(obj->source.file) == EOF) {
            fprintf(stderr, "FATAL: Error occurred while closing file '%s'\n", obj->source.path);
            longjmp(obj->jmp, 1); /* never returns */
        }
    }
    obj->source.path = NULL;
    obj->source.file = NULL;
}

int system__read_source_file(system_t *obj) {
    const int c = fgetc(obj->source.file);
    if (c != EOF) {
        append_text_character_(obj, (char)c);
        if (c == '\r') {
            append_line_head_(obj, obj->source.text.n);
        }
        else if (c == '\n') {
            if (obj->source.text.n >= 2 && obj->source.text.p[obj->source.text.n - 2] == '\r') {
                obj->source.line.p[obj->source.line.n - 1] = obj->source.text.n;
            }
            else {
                append_line_head_(obj, obj->source.text.n);
            }
        }
    }
    else {
        if (ferror(obj->source.file)) {
            if (obj->source.path != NULL) {
                fprintf(stderr, "FATAL: Error occurred while reading file '%s'\n", obj->source.path);
            }
            else {
                fprintf(stderr, "FATAL: Error occurred while reading standard input\n");
            }
            longjmp(obj->jmp, 1); /* never returns */
        }
    }
    return c;
}

static ast_node_t *create_ast_node_(system_t *obj, ast_node_type_t type, range_t range) {
    ast_node_t *const node = (ast_node_t *)system__allocate_memory(obj, sizeof(ast_node_t));
    node->type = type;
    node->range = range;
    node->arity = 0;
    node->parent = NULL;
    node->sibling.prev = NULL;
    node->sibling.next = NULL;
    node->child.first = NULL;
    node->child.last = NULL;
    node->system = obj;
    if (obj->managed.last != NULL) {
        node->managed.prev = obj->managed.last;
        node->managed.next = NULL;
        obj->managed.last = node;
        if (node->managed.prev != NULL) {
            node->managed.prev->managed.next = node;
        }
        else {
            obj->managed.first = node;
        }
    }
    else {
        node->managed.prev = NULL;
        node->managed.next = NULL;
        obj->managed.first = node;
        obj->managed.last = node;
    }
    return node;
}

ast_node_t *system__create_ast_node_terminal(system_t *obj, ast_node_type_t type, range_t range) {
    return create_ast_node_(obj, type, range);
}

ast_node_t *system__create_ast_node_unary(system_t *obj, ast_node_type_t type, range_t range, ast_node_t *node1) {
    if (node1 == NULL) {
        fprintf(stderr, "FATAL: Internal error\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    ast_node_t *const node = create_ast_node_(obj, type, range);
    ast_node__append_child(node, node1);
    return node;
}

ast_node_t *system__create_ast_node_binary(system_t *obj, ast_node_type_t type, range_t range, ast_node_t *node1, ast_node_t *node2) {
    if (node1 == NULL || node2 == NULL) {
        fprintf(stderr, "FATAL: Internal error\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    ast_node_t *const node = create_ast_node_(obj, type, range);
    ast_node__append_child(node, node1);
    ast_node__append_child(node, node2);
    return node;
}

ast_node_t *system__create_ast_node_ternary(system_t *obj, ast_node_type_t type, range_t range, ast_node_t *node1, ast_node_t *node2, ast_node_t *node3) {
    if (node1 == NULL || node2 == NULL || node3 == NULL) {
        fprintf(stderr, "FATAL: Internal error\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    ast_node_t *const node = create_ast_node_(obj, type, range);
    ast_node__append_child(node, node1);
    ast_node__append_child(node, node2);
    ast_node__append_child(node, node3);
    return node;
}

ast_node_t *system__create_ast_node_variadic(system_t *obj, ast_node_type_t type, range_t range) {
    return create_ast_node_(obj, type, range);
}

void system__destroy_all_ast_nodes(system_t *obj) {
    while (obj->managed.first != NULL) {
        ast_node__destroy(obj->managed.first);
    }
}

void ast_node__prepend_child(ast_node_t *obj, ast_node_t *node) {
    if (node == NULL) return; /* just ignored */
    if (node->parent != NULL) {
        if (node->sibling.prev != NULL) {
            node->sibling.prev->sibling.next = node->sibling.next;
        }
        else {
            node->parent->child.first = node->sibling.next;
        }
        if (node->sibling.next != NULL) {
            node->sibling.next->sibling.prev = node->sibling.prev;
        }
        else {
            node->parent->child.last = node->sibling.prev;
        }
        node->parent->arity--;
    }
    node->parent = obj;
    if (obj->child.first != NULL) {
        obj->child.first->sibling.prev = node;
        node->sibling.next = obj->child.first;
        node->sibling.prev = NULL;
        obj->child.first = node;
    }
    else {
        node->sibling.next = NULL;
        node->sibling.prev = NULL;
        obj->child.first = node;
        obj->child.last = node;
    }
    obj->arity++;
}

void ast_node__append_child(ast_node_t *obj, ast_node_t *node) {
    if (node == NULL) return; /* just ignored */
    if (node->parent != NULL) {
        if (node->sibling.prev != NULL) {
            node->sibling.prev->sibling.next = node->sibling.next;
        }
        else {
            node->parent->child.first = node->sibling.next;
        }
        if (node->sibling.next != NULL) {
            node->sibling.next->sibling.prev = node->sibling.prev;
        }
        else {
            node->parent->child.last = node->sibling.prev;
        }
        node->parent->arity--;
    }
    node->parent = obj;
    if (obj->child.last != NULL) {
        obj->child.last->sibling.next = node;
        node->sibling.prev = obj->child.last;
        node->sibling.next = NULL;
        obj->child.last = node;
    }
    else {
        node->sibling.prev = NULL;
        node->sibling.next = NULL;
        obj->child.last = node;
        obj->child.first = node;
    }
    obj->arity++;
}

void ast_node__destroy(ast_node_t *obj) {
    if (obj->parent != NULL) {
        if (obj->sibling.prev != NULL) {
            obj->sibling.prev->sibling.next = obj->sibling.next;
        }
        else {
            obj->parent->child.first = obj->sibling.next;
        }
        if (obj->sibling.next != NULL) {
            obj->sibling.next->sibling.prev = obj->sibling.prev;
        }
        else {
            obj->parent->child.last = obj->sibling.prev;
        }
        obj->parent->arity--;
    }
    while (obj->child.first != NULL) {
        ast_node_t *const child = obj->child.first;
        obj->child.first = child->sibling.next;
        child->sibling.next = NULL;
        child->sibling.prev = NULL;
        child->parent = NULL;
    }
    {
        if (obj->managed.prev != NULL) {
            obj->managed.prev->managed.next = obj->managed.next;
        }
        else {
            obj->system->managed.first = obj->managed.next;
        }
        if (obj->managed.next != NULL) {
            obj->managed.next->managed.prev = obj->managed.prev;
        }
        else {
            obj->system->managed.last = obj->managed.prev;
        }
    }
    system__deallocate_memory(obj->system, obj);
}

static void dump_ast_(system_t *obj, ast_node_t *node, int level) {
    const char *type = "UNKNOWN";
    switch (node->type) {
    case AST_NODE_TYPE_GRAMMAR:             type = "GRAMMAR";             break;
    case AST_NODE_TYPE_RULE:                type = "RULE";                break;
    case AST_NODE_TYPE_DIRECTIVE:           type = "DIRECTIVE";           break;
    case AST_NODE_TYPE_DIRECTIVE_NAME:      type = "DIRECTIVE_NAME";      break;
    case AST_NODE_TYPE_CODE:                type = "CODE";                break;
    case AST_NODE_TYPE_SOURCE:              type = "SOURCE";              break;
    case AST_NODE_TYPE_PRIMARY:             type = "PRIMARY";             break;
    case AST_NODE_TYPE_ALTERNATION:         type = "ALTERANTION";         break;
    case AST_NODE_TYPE_SEQUENCE:            type = "SEQUENCE";            break;
    case AST_NODE_TYPE_PREFIX_OP:           type = "PREFIX_OP";           break;
    case AST_NODE_TYPE_POSTFIX_OP:          type = "POSTFIX_OP";          break;
    case AST_NODE_TYPE_RULEREF:             type = "RULEREF";             break;
    case AST_NODE_TYPE_VAR:                 type = "VAR";                 break;
    case AST_NODE_TYPE_STRING:              type = "STRING";              break;
    case AST_NODE_TYPE_CHARCLASS:           type = "CHARCLASS";           break;
    case AST_NODE_TYPE_DOT:                 type = "DOT";                 break;
    case AST_NODE_TYPE_BACKREF:             type = "BACKREF";             break;
    case AST_NODE_TYPE_GROUP:               type = "GROUP";               break;
    case AST_NODE_TYPE_CAPTURE:             type = "CPATURE";             break;
    default: break;
    }
    if (node->arity > 0) {
        printf("%*s%s: arity = %zu\n", 2 * level, "", type, node->arity);
        for (ast_node_t *p = node->child.first; p != NULL; p = p->sibling.next) {
            dump_ast_(obj, p, level + 1);
        }
    }
    else {
        size_t line, col;
        compute_line_and_column_(obj, node->range.min, &line, &col);
        printf(
            "%*s%s: line = %zu, column = %zu, value = '%.*s'\n",
            2 * level, "", type, line, col,
            (int)(node->range.max - node->range.min), obj->source.text.p + node->range.min
        );
    }
}

void system__dump_ast(system_t *obj, ast_node_t *root) {
    dump_ast_(obj, root, 0);
}
