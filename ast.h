#ifndef INCLUDED_AST_H
#define INCLUDED_AST_H

#include "c_api.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

struct AstNode {
private:
    std::string text;
    AstNodeType type;
    int line;
    int column;

    std::vector<AstNode*> children;
    AstNode* parent;

    AstNode* find_prev_sibling() const;
    AstNode* find_next_sibling() const;
    AstNode* find_parent(AstNodeType type) const;
    std::vector<AstNode*> find_all(const std::function <bool(const AstNode&)>& predicate, const bool global = false) const;

    const char* get_type_name() const;

    void remove_child(AstNode* child);
    void replace_child(AstNode* removed, AstNode* added, bool do_delete = true);

    bool is_terminal() const;

    bool operator==(const AstNode& x) const;
    bool operator!=(const AstNode& x) const;

    void format_terminal() const;
    void format_grammar() const;
    void format_string() const;
    void format_character_class() const;
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
    int optimize_single_child();
    int optimize_strings();
    int optimize_character_class();
    int optimize_repeats();
    int optimize_strip_comment();
    int optimize_inline_rule();
    int optimize_unused_variable();
    int optimize_unused_captures();
    int optimize_children();

public:
    void append_child(AstNode* node);
    void print_ast(int level = 0) const;
    void format() const;
    int optimize();
    void debug();
    void stats(const char* prefix, bool force = false) const;

    AstNode(AstNodeType type, std::string text = "", size_t line = -1, size_t column=-1);
    AstNode(const AstNode& other, AstNode* parent);
    ~AstNode();
};

#endif /* INCLUDED_AST_H */
