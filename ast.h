#ifndef INCLUDED_AST_H
#define INCLUDED_AST_H

#include "c_api.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

using std::string;
using std::vector;

struct AstNode {
private:
    string text;
    AstNodeType type;
    int line;
    int column;

    vector<AstNode*> children;
    AstNode* parent;

    AstNode* find_parent(AstNodeType type) const;
    vector<AstNode*> find_all(const std::function <bool(const AstNode&)>& predicate, const bool global = true) const;

    const char* get_type_name() const;

    void remove_child(AstNode* child);

    void format_terminal() const;
    void format_grammar() const;
    void format_string() const;
    void format_source() const;
    void format_error() const;
    void format_directive() const;
    void format_alternation() const;
    void format_sequence() const;
    void format_primary() const;
    void format_ruleref() const;
    void format_rule() const;
    void format_code() const;
    void format_comment() const;
    void format_group(const char open, const char close) const;

    int optimize_grammar();
    int optimize_sequence();
    int optimize_alternation();
    int optimize_primary();
    int optimize_rule();

    int optimize_single_child();
    int optimize_strings();
    int optimize_strip_comment();
    int optimize_inline_rule();
    int optimize_children();

public:
    AstNode(AstNodeType type, string text = "", size_t line = -1, size_t column=-1) :
        text(text), type(type), line(line), column(column), parent(NULL)
    {
        //~ printf("CREATE %p (size=%ld, parent=%p, type=%s)\n", this, children.size(), parent, getTypeName());
    }

    AstNode(const AstNode& other, AstNode* parent) :
        text(other.text), type(other.type), line(other.line), column(other.column), parent(parent)
    {
        for (int i = 0; i < other.children.size(); i++) {
            children.push_back(new AstNode(*other.children[i], this));
        }
        //~ printf("COPY %p -> %p (size=%ld, parent=%p, type=%s)\n", this, &other, children.size(), parent, getTypeName());
    }

    ~AstNode() {
        //~ printf("DELETE %p (size=%ld, parent=%p, type=%s)\n", this, children.size(), parent, getTypeName());
        for (size_t i = 0; i < children.size(); i++) {
            delete children[i];
        }
    }

    void append_child(AstNode* node);
    void print_ast(int level = 0) const;
    void format() const;
    int optimize();
};

#endif /* INCLUDED_AST_H */
