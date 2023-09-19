#include "node.h"

Node::Node(const char* type, Node* parent) : valid(false), parent(parent), type(type) {
    //~ printf("DBG: creating %s\n", type);
}

Node::operator bool() {
    return valid;
}

Node* Node::operator[](int index) {
    return this;
}

long Node::size() const {
    return 1;
}

void Node::parse_comments(Parser2& p) {
    if (p.match_comment()) {
        comment = p.last_match;
    }
}

std::string Node::comments(std::string indent) const {
    if (comment.empty()) return "";
    return indent + "# " + std::regex_replace(comment, std::regex("\n"), "\n" + indent + "# ") + "\n";
}
