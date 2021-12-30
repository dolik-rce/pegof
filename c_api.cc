#include "c_api.h"
#include "ast.h"
#include "source.h"

int source_read(SourceC source) {
    Source* s = reinterpret_cast<Source*>(source);
    if (!s) {
        fprintf(stderr, "FATAL: Internal error.\n");
        exit(12);
    }
    return s->read();
}

AstNodeC create_ast_leaf(SourceC source, AstNodeType type, const char* text, size_t start) {
    size_t line = -1;
    size_t col = -1;
    if (start >= 0) {
        Source* s = reinterpret_cast<Source*>(source);
        std::pair<size_t, size_t> pos = s->compute_position(start);
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
