#include "ast.h"
#include "io.h"

void AstNode::append_child(AstNode* node) {
    children.push_back(node);
    node->parent = this;
}

bool AstNode::operator!=(const AstNode& x) const {
    return !(*this == x);
}

bool AstNode::operator==(const AstNode& x) const {
    if (type != x.type) {
        return false;
    }
    if (text != x.text) {
        return false;
    }
    if (children.size() != x.children.size()) {
        return false;
    }
    for (size_t i = 0; i < children.size(); i++) {
        if (*children[i] != *x.children[i]) {
            return false;
        }
    }
    return true;
}

const char* AstNode::get_type_name() const {
    switch (type) {
    case AST_GRAMMAR:           return "GRAMMAR";
    case AST_COMMENT:           return "COMMENT";
    case AST_RULE:              return "RULE";
    case AST_RULE_NAME:         return "RULE_NAME";
    case AST_DIRECTIVE:         return "DIRECTIVE";
    case AST_DIRECTIVE_NAME:    return "DIRECTIVE_NAME";
    case AST_CODE:              return "CODE";
    case AST_SOURCE:            return "SOURCE";
    case AST_ERROR:             return "ERROR";
    case AST_PRIMARY:           return "PRIMARY";
    case AST_ALTERNATION:       return "ALTERNATION";
    case AST_SEQUENCE:          return "SEQUENCE";
    case AST_PREFIX_OP:         return "PREFIX_OP";
    case AST_POSTFIX_OP:        return "POSTFIX_OP";
    case AST_RULEREF:           return "RULEREF";
    case AST_REFNAME:           return "REFNAME";
    case AST_VAR:               return "VAR";
    case AST_STRING:            return "STRING";
    case AST_CHARCLASS:         return "CHARCLASS";
    case AST_DOT:               return "DOT";
    case AST_BACKREF:           return "BACKREF";
    case AST_GROUP:             return "GROUP";
    case AST_CAPTURE:           return "CAPTURE";
    default:
        Io::debug("%s\n", "ERROR: unexpected AST node type!");
        exit(2);
    }
}

void AstNode::print_ast(int level) const {
    const char* typeName = get_type_name();

    if (children.size() > 0) {
        Io::print("%*s%s:\n", 2*level, "", typeName);
        for (int i=0; i < children.size(); i++) {
            children[i]->print_ast(level + 1);
        }
    } else if (line >= 0) {
        Io::print("%*s%s: \"%s\" line=%ld, col=%ld\n", 2*level, "", typeName, text.c_str(), line, column);
    } else {
        Io::print("%*s%s: \"%s\" (optimized)\n", 2*level, "", typeName, text.c_str(), line, column);
    }
}

AstNode* AstNode::find_prev_sibling() const {
    if (!parent) {
        return NULL;
    }
    const std::vector<AstNode*>::iterator self = std::find(parent->children.begin(), parent->children.end(), this);
    if (self == parent->children.end() || self == parent->children.begin()) {
        return NULL;
    }
    return *(self - 1);
}

AstNode* AstNode::find_next_sibling() const {
    if (!parent) {
        return NULL;
    }
    const std::vector<AstNode*>::iterator self = std::find(parent->children.begin(), parent->children.end(), this);
    if (self == parent->children.end() || self + 1 == parent->children.end()) {
        return NULL;
    }
    return *(self + 1);
}

AstNode* AstNode::find_parent(AstNodeType type) const {
    if (!parent) {
        return NULL;
    } else if (parent->type == type) {
        return parent;
    } else {
        return parent->find_parent(type);
    }
}

std::vector<AstNode*> AstNode::find_all(const std::function <bool(const AstNode&)>& predicate, const bool global) const {
    if (global) {
        return find_parent(AST_GRAMMAR)->find_all(predicate);
    }
    std::vector<AstNode*> result;
    if (predicate(*this)) {
        result.push_back(const_cast<AstNode *>(this));
    }
    for (int i = 0; i < children.size(); i++) {
        std::vector<AstNode*> subresults = children[i]->find_all(predicate);
        if (!subresults.empty()) {
            result.insert(result.end(), subresults.begin(), subresults.end());
        }
    }
    return result;
}

void AstNode::remove_child(AstNode* child) {
    children.erase(std::find(children.begin(), children.end(), child));
    delete child;
}

void AstNode::replace_child(AstNode* removed, AstNode* added, bool do_delete) {
    std::vector<AstNode*>::iterator pos = std::find(children.begin(), children.end(), removed);
    *pos = added;
    added->parent = this;
    if (do_delete) {
        delete removed;
    }
}

void AstNode::debug() {
    Io::print("%s\n", "### Original AST:");
    print_ast();
    Io::print("%s\n", "### Original formatted:");
    format();
    optimize();
    Io::print("%s\n", "### Optimized AST:");
    print_ast();
    Io::print("%s\n", "### Optimized formatted:");
    format();
}

AstNode::AstNode(AstNodeType type, std::string text, size_t line, size_t column) :
    text(text), type(type), line(line), column(column), parent(NULL) {}

AstNode::AstNode(const AstNode& other, AstNode* parent) :
    text(other.text), type(other.type), line(other.line), column(other.column), parent(parent)
{
    for (int i = 0; i < other.children.size(); i++) {
        children.push_back(new AstNode(*other.children[i], this));
    }
}

AstNode::~AstNode() {
    for (size_t i = 0; i < children.size(); i++) {
        delete children[i];
    }
}
