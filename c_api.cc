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
