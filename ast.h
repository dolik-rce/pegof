#ifndef INCLUDED_AST_H
#define INCLUDED_AST_H

#include "c_api.h"

#include <string>
#include <vector>
#include <memory>

using std::string;
using std::vector;

struct AstNode {
private:
    string text;
    ast_node_type_t type;
    int line;
    int column;

    vector<AstNode*> children;
    AstNode* parent;

    AstNode* find_parent(ast_node_type_t);

    const char* getTypeName();

    void format_terminal();
    void format_grammar();
    void format_string();
    void format_source();
    void format_directive();
    void format_alternation();
    void format_sequence();
    void format_primary();
    void format_ruleref();
    void format_rule();
    void format_code();
    void format_comment();
    void format_group(const char open, const char close);

    int optimize_grammar();
    int optimize_sequence();
    int optimize_alternation();
    int optimize_primary();

    int optimize_single_child();
    int optimize_strings();
    int optimize_strip_comment();
    int optimize_children();

public:
    AstNode(ast_node_type_t type, string text = "", size_t line = -1, size_t column=-1):
        text(text), type(type), line(line), column(column) {}

    ~AstNode() {
        for (size_t i = 0; i < children.size(); i++) {
            delete children[i];
        }
    }

    void appendChild(AstNode* node);
    void print_ast(int level = 0);
    void format();
    int optimize();
};

#endif /* INCLUDED_AST_H */
