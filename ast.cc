#include "ast.h"

void AstNode::appendChild(AstNode* node) {
    children.push_back(node);
    node->parent = this;
}

void AstNode::prependChild(AstNode* node) {
    children.insert(children.begin(), node);
    node->parent = this;
}

const char* AstNode::getTypeName() {
    switch (type) {
    case AST_NODE_TYPE_GRAMMAR:             return "GRAMMAR";
    case AST_NODE_TYPE_COMMENT:             return "COMMENT";
    case AST_NODE_TYPE_RULE:                return "RULE";
    case AST_NODE_TYPE_RULE_NAME:           return "RULE_NAME";
    case AST_NODE_TYPE_DIRECTIVE:           return "DIRECTIVE";
    case AST_NODE_TYPE_DIRECTIVE_NAME:      return "DIRECTIVE_NAME";
    case AST_NODE_TYPE_CODE:                return "CODE";
    case AST_NODE_TYPE_SOURCE:              return "SOURCE";
    case AST_NODE_TYPE_PRIMARY:             return "PRIMARY";
    case AST_NODE_TYPE_ALTERNATION:         return "ALTERNATION";
    case AST_NODE_TYPE_SEQUENCE:            return "SEQUENCE";
    case AST_NODE_TYPE_PREFIX_OP:           return "PREFIX_OP";
    case AST_NODE_TYPE_POSTFIX_OP:          return "POSTFIX_OP";
    case AST_NODE_TYPE_RULEREF:             return "RULEREF";
    case AST_NODE_TYPE_VAR:                 return "VAR";
    case AST_NODE_TYPE_STRING:              return "STRING";
    case AST_NODE_TYPE_CHARCLASS:           return "CHARCLASS";
    case AST_NODE_TYPE_DOT:                 return "DOT";
    case AST_NODE_TYPE_BACKREF:             return "BACKREF";
    case AST_NODE_TYPE_GROUP:               return "GROUP";
    case AST_NODE_TYPE_CAPTURE:             return "CAPTURE";
    default:                                return "UNKNOWN";
    }
}

void AstNode::print_ast(int level) {
    const char* typeName = getTypeName();

    if (children.size() > 0) {
        printf("%*s%s:\n", 2*level, "", typeName);
        //~ printf("%*sthis=%p, parent=%p, children=%ld\n", 2*level, "", this, parent, children.size());
        for (int i=0; i < children.size(); i++) {
            children[i]->print_ast(level + 1);
        }
    } else if (line >= 0) {
        printf("%*s%s: \"%s\" line=%ld, col=%ld\n", 2*level, "", typeName, text.c_str(), line, column);
        //~ printf("%*sthis=%p, parent=%p, children=%ld\n", 2*level, "", this, parent, children.size());
    } else {
        printf("%*s%s: \"%s\" (optimized)\n", 2*level, "", typeName, text.c_str(), line, column);
        //~ printf("%*sthis=%p, parent=%p, children=%ld\n", 2*level, "", this, parent, children.size());
    }
}

void AstNode::format_terminal() {
    printf("%s", text.c_str());
}

void AstNode::format_grammar() {
    if (children.size() > 0) {
        for (size_t i = 0; i< children.size(); i++) {
            children[i]->format();
        }
    }
}

void AstNode::format_string() {
    // TODO: configurable quotes
    printf("\"");
    char next = 0;
    for(size_t i = 0; i < text.size(); i++) {
        switch (text[i]) {
        case '\\':
            next = text[++i];
            printf("%s%c", next == '\'' ? "" : "\\", next);
            break;
        case '"':
            printf("\\\"");
            break;
        default:
            printf("%c", text[i]);
            break;
        }
    }
    printf("\"");
}

void AstNode::format_source() {
    size_t first_non_ws = 0;
    while (isspace(text[first_non_ws])) ++first_non_ws;
    size_t last_non_ws = text.size();
    while (isspace(text[--last_non_ws])) {};
    last_non_ws++;

    bool hasNewlines = text.find_first_of('\n') != string::npos;

    int is_directive = parent->type == AST_NODE_TYPE_DIRECTIVE;

    if (hasNewlines) {
        printf(" {%s}", text.c_str());
    } else {
        size_t base_indent = 0; //TODO: check that no line has indent less than base_indent
        while (text[first_non_ws - base_indent - 1] != '\n') ++base_indent;
        printf(" {\n");

        printf(is_directive ? "    " : "        ");
        size_t pos = first_non_ws;
        while (pos < last_non_ws) {
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

void AstNode::format_directive() {
    AstNode* content = children[1];

    printf("%%");
    children[0]->format();
    if (content->type == AST_NODE_TYPE_STRING) {
        printf(" ");
    }
    content->format();
    printf("\n\n");
}

void AstNode::format_alternation() {
    // TODO: configurable wrapping (higher children.size() treshold)
    int multiline = children.size() > 1 && parent->type == AST_NODE_TYPE_RULE;
    const char *delim = multiline ? "\n    / " : " / ";
    for (size_t i = 0; i < children.size(); i++) {
        if (i > 0) {
            printf("%s", delim);
        }
        children[i]->format();
    }
}

void AstNode::format_sequence() {
    for (size_t i = 0; i < children.size(); i++) {
        if (i > 0) {
            printf(" ");
        }
        children[i]->format();
    }
}

void AstNode::format_primary() {
    for (size_t i = 0; i < children.size(); i++) {
        children[i]->format();
    }
}

void AstNode::format_group(const char open, const char close) {
    printf("%c", open);
    children[0]->format();
    printf("%c", close);
}

void AstNode::format_ruleref() {
    if (children.size() > 0){
        // has variable
        children[0]->format();
        printf(":");
    }
    printf("%s", text.c_str());
}

void AstNode::format_rule() {
    AstNode* body = children[1];
    bool hasAlternation = body->type == AST_NODE_TYPE_ALTERNATION && body->children.size() > 1;
    printf("%s%s <- ", children[0]->text.c_str(), hasAlternation ? "\n   " : "");
    body->format();
    printf("\n\n");
}

void AstNode::format_code() {
    size_t last_non_ws = text.size();
    while (isspace(text[--last_non_ws])) {};
    printf("%%%%\n%.*s\n", last_non_ws + 1, text.c_str());
}

void AstNode::format_comment() {
    // We have to recognize three different comment positions:
    //   1) top-level comment (parent is GRAMMAR) -> add newline, no indent
    //   2) at the end of sequence in top-lefel alternation -> no newline, no indent
    //   3) anywhere else -> add newline + indent
    const char* suffix = "\n        ";
    if (parent->type == AST_NODE_TYPE_GRAMMAR) {
        suffix = "\n";
    } else {
        AstNode* parent_alternation = find_parent(AST_NODE_TYPE_ALTERNATION);
        if (parent_alternation
            && parent_alternation->parent->type == AST_NODE_TYPE_RULE
            && parent->parent->children.back() == parent /* parent is last sibling*/)
        {
            suffix = "";
        }
    }

    size_t last_non_ws = text.size();
    while (isspace(text[--last_non_ws])) {};
    printf("# %.*s%s", last_non_ws + 1, text, suffix);
}

AstNode* AstNode::find_parent(ast_node_type_t type) {
    if (!parent) {
        return parent;
    } else if (parent->type == type) {
        return parent;
    } else {
        return parent->find_parent(type);
    }
}

void AstNode::format() {
    switch (type) {
    case AST_NODE_TYPE_GRAMMAR:         format_grammar(); break;
    case AST_NODE_TYPE_COMMENT:         format_comment(); break;
    case AST_NODE_TYPE_RULE:            format_rule(); break;
    case AST_NODE_TYPE_DIRECTIVE:       format_directive(); break;
    case AST_NODE_TYPE_CODE:            format_code(); break;
    case AST_NODE_TYPE_PRIMARY:         format_primary(); break;
    case AST_NODE_TYPE_ALTERNATION:     format_alternation(); break;
    case AST_NODE_TYPE_SEQUENCE:        format_sequence(); break;
    case AST_NODE_TYPE_RULEREF:         format_ruleref(); break;
    case AST_NODE_TYPE_GROUP:           format_group('(', ')'); break;
    case AST_NODE_TYPE_CAPTURE:         format_group('<', '>'); break;
    case AST_NODE_TYPE_SOURCE:          format_source(); break;
    case AST_NODE_TYPE_STRING:          format_string(); break;

    // everything else just print verbatim:
    case AST_NODE_TYPE_RULE_NAME:
    case AST_NODE_TYPE_DIRECTIVE_NAME:
    case AST_NODE_TYPE_VAR:
    case AST_NODE_TYPE_PREFIX_OP:
    case AST_NODE_TYPE_POSTFIX_OP:
    case AST_NODE_TYPE_CHARCLASS:
    case AST_NODE_TYPE_DOT:
    case AST_NODE_TYPE_BACKREF:
        format_terminal();
        break;
    }
}

int AstNode::optimize_strings() {
    int optimized = 0;
    for (size_t i = 1; i < children.size(); i++) {
        AstNode* first = children[i-1];
        AstNode* second = children[i];
        if (first->type == AST_NODE_TYPE_STRING && second->type == AST_NODE_TYPE_STRING) {
            first->text += second->text;
            first->line = -2;
            first->column = -2;
            delete second;
            children.erase(children.begin() + i);
            optimized++;
            i = 1; // try again from start
        }
    }
    return optimized;
}

int AstNode::optimize_single_child() {
    if (children.size() != 1) {
        return 0;
    }
    AstNode* child = children[0];
    //~ printf("OPT[this]: this=%p, parent=%p %s:'%s'\n", this, parent, getTypeName(), text.c_str());
    //~ for (size_t i=0;i<children.size();i++) printf("  child[%ld]=%p\n", i, children[i]);
    //~ printf("OPT[child]: this=%p, parent=%p %s:'%s'\n", child, child->parent, child->getTypeName(), child->text.c_str());
    //~ for (size_t i=0;i<child->children.size();i++) printf("  child[%ld]=%p\n", i, child->children[i]);
    text = child->text;
    type = child->type;
    line = child->line;
    column = child->column;
    children = child->children;
    child->children.clear(); // clear children to avoid their recursive deletion
    delete child;
    //~ printf("OPT[final]: this=%p, parent=%p %s:'%s'\n", this, parent, getTypeName(), text.c_str());
    //~ for (size_t i=0;i<children.size();i++) printf("  child[%ld]=%p\n", i, children[i]);
    return 1;
}

int AstNode::optimize_children() {
    int optimized = 0;
    for (size_t i = 0; i < children.size(); i++) {
        optimized += children[i]->optimize();
    }
    return optimized;
}

int AstNode::optimize_grammar() {
    int total = 0;
    int optimized;
    int i = 0;
    do {
        optimized = optimize_children();
        total += optimized;
        printf("OPTIMIZING (pass %d): %d optimizations done (%d total)\n", ++i, optimized, total);
    } while (optimized > 0);
    return total;
}

int AstNode::optimize_sequence() {
    return optimize_single_child() + optimize_strings() + optimize_children();
}

int AstNode::optimize_alternation() {
    return optimize_single_child() + optimize_children();
}

int AstNode::optimize_primary() {
    return optimize_single_child() + optimize_children();
}

int AstNode::optimize() {
    switch (type) {
    case AST_NODE_TYPE_GRAMMAR:         return optimize_grammar();
    case AST_NODE_TYPE_PRIMARY:         return optimize_primary();
    case AST_NODE_TYPE_ALTERNATION:     return optimize_alternation();
    case AST_NODE_TYPE_SEQUENCE:        return optimize_sequence();

    // everything else just call optimize on children
    case AST_NODE_TYPE_COMMENT:
    case AST_NODE_TYPE_RULE:
    case AST_NODE_TYPE_DIRECTIVE:
    case AST_NODE_TYPE_CODE:
    case AST_NODE_TYPE_SOURCE:
    case AST_NODE_TYPE_STRING:
    case AST_NODE_TYPE_RULEREF:
    case AST_NODE_TYPE_GROUP:
    case AST_NODE_TYPE_CAPTURE:
    case AST_NODE_TYPE_RULE_NAME:
    case AST_NODE_TYPE_DIRECTIVE_NAME:
    case AST_NODE_TYPE_VAR:
    case AST_NODE_TYPE_PREFIX_OP:
    case AST_NODE_TYPE_POSTFIX_OP:
    case AST_NODE_TYPE_CHARCLASS:
    case AST_NODE_TYPE_DOT:
    case AST_NODE_TYPE_BACKREF:
        return optimize_children();
    }
    throw "ERROR: unexpected AST node type!";
}

//~ #include <stdio.h>
//~ #include <stdlib.h>
//~ #include <sys/wait.h>
//~ #include <unistd.h>
//~ void print_stack() {
    //~ char pid_buf[30];
    //~ sprintf(pid_buf, "--pid=%d", getpid());
    //~ char name_buf[512];
    //~ name_buf[readlink("/proc/self/exe", name_buf, 511)]=0;
    //~ int child_pid = fork();
    //~ if (!child_pid) {
        //~ dup2(2,1); // redirect output to stderr
        //~ fprintf(stdout,"stack trace for %s pid=%s\n",name_buf,pid_buf);
        //~ execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
        //~ abort(); /* If gdb failed to start */
    //~ } else {
        //~ waitpid(child_pid,NULL,0);
    //~ }
//~ }
