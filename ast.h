#ifndef INCLUDED_AST_H
#define INCLUDED_AST_H

#include "c_api.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <variant>

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
    int optimize_negation();
    int optimize_repeats();
    int optimize_double_postfix();
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

struct Grammar;
struct Directive;
struct Error;
struct Action;
struct Expand;
struct Capture;
struct Alternation;
struct Sequence;
struct Predicate;
struct Quantity;
struct CharClass;
struct String;
struct Reference;
struct Rule;
struct Comment;

typedef std::variant<
    std::vector<void*>,
    Grammar, Directive, Error, Action, Expand, Capture, Alternation, Sequence,
    Predicate, Quantity, CharClass, String, Reference, Rule, Comment
> AnyNode;

struct Node {
    const char* type;
    Node(const char* type): type(type) {}
    std::string to_string() { return "TODO"; }
};

struct WithPlacement {
    int line;
    int column;
};

struct WithExpession {
    Alternation *expression;
};

struct Comment : Node, WithPlacement {
    std::string text;
    Comment(const char* text): Node("COMMENT"), text(text) {}
};


struct Rule : Node, WithPlacement, WithExpession {
    std::string name;
    Rule(const char* name): Node("Rule"), name(name) {}
    std::string to_string() {
        return std::string("Rule '") + name + "' [" + std::to_string(line) + ":" + std::to_string(column) + "]";
    }
};

struct Reference : Node, WithPlacement {
    std::string variable;
    int index;
};

struct String : Node {
    std::string value;
};

struct CharClass : Node {
    // TODO: rename or merge with Characterstruct
    std::string value;
};

struct Quantity : Node, WithExpession {
    int min;
    int max;
};

struct Predicate : Node, WithExpession {
    bool negative;
};

struct Sequence : Node {
    std::vector<Alternation*> children;
};

struct Alternation : Node {
    std::vector<Alternation*> children;
};

struct Capture : Node, WithExpession {
    int index;
};

struct Expand : Node, WithPlacement {
    int index;
};

struct Action : Node {
    int index;
    std::string code;
};

struct Error : Node, WithExpession {
    int index;
    std::string code;
};

struct Directive : Node {
    std::string name;
    std::string code;
    Directive(const char* name, const char* code) : Node("DIRECTIVE"), name(name), code(code) {}
};

struct Grammar : Node {
    std::vector<AnyNode*> children;
    std::string code;
    Grammar(const std::vector<void*>& nodes) : Node("GRAMMAR") {
        for (const auto& child : nodes) {
            children.push_back((AnyNode*) child);
        }
    }
    std::string to_string() {
        std::string s = "Grammmar:\n";
        for (const auto& child : children) {
            s += ((Rule*)child)->to_string() + "\n";
        }
        return s;
    }
};

#endif /* INCLUDED_AST_H */
