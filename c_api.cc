#include "c_api.h"
#include "ast.h"
#include "io.h"

int source_read(void *) {
    return Io::read();
}

AstNodeC create_ast_leaf(AstNodeType type, const char* text, size_t start) {
    size_t line = -1;
    size_t col = -1;
    if (start >= 0) {
        std::pair<size_t, size_t> pos = Io::compute_position(start);
        line = pos.first;
        col = pos.second;
    }
    return new AstNode(type, std::string(text), line, col);
}

AstNodeC create_ast_node(AstNodeType type) {
    return new AstNode(type);
}

void append_child(AstNodeC parent, AstNodeC child) {
    AstNode *p = reinterpret_cast<AstNode*>(parent);
    AstNode *c = reinterpret_cast<AstNode*>(child);
    if (!c) return;
    p->append_child(c);
}

AstNodeC list_create(void* item) {
    printf("DBG: creating list\n");
    return new AnyNode(std::vector<void*>(1, item));
}

AstNodeC list_append(void* list, void* item) {
    printf("DBG: adding to list\n");
    std::vector<void*>& v = std::get<std::vector<void*>>(*(AnyNode*) list);
    v.push_back(item);
    return list;
}

AstNodeC create_grammar(AstNodeC nodes) {
    std::vector<void*>& children = std::get<std::vector<void*>>(*(AnyNode*) nodes);
    printf("DBG: grammar %d\n", children.size());
    return new AnyNode(Grammar(children));
}

AstNodeC create_directive(const char* name, const char* content) {
    printf("DBG: directive %s\n", name);
    return new AnyNode(Directive(name, content));
}

AstNodeC create_rule(const char* name) {
    printf("DBG: rule %s\n", name);
    return new AnyNode(Rule(name)); //TODO: expression
}

AstNodeC create_comment(const char* text) {
    printf("DBG: comment %s\n", text);
    return new AnyNode(Comment(text));
}
