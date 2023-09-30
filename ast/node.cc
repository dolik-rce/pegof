#include "node.h"
#include "log.h"

Node::Node(const char* type, Node* parent) : valid(false), parent(parent), type(type) {
    debug("Creating %s @%p, parent: %p", type, this, parent);
}

Node::operator bool() const {
    return valid;
}

Node* Node::operator[](int index) {
    return nullptr;
}

long Node::size() const {
    return 0;
}

void Node::parse_comments(Parser& p) {
    debug("Parsing comments for node of type %s", type);
    p.skip_space();
    while (p.match_comment()) {
        comments.push_back(p.last_match);
        debug("Comment: '%s'", comments.back().c_str());
    }
}

std::string Node::format_comments(std::string indent) const {
    if (comments.empty()) return "";
    std::string result;
    for(int i = 0; i < comments.size(); i++) {
        result += (i ? "\n" : "") + indent + "# " + comments[i];
    }
    return result;
}

void Node::map(const std::function<bool(Node&)>& transform) {
    if (transform(*this)) return;
    for (int i = 0; i < size(); i++) {
        Node* n = (*this)[i];
        n->map(transform);
    }
}

void Node::update_parents() {
    for (int i = 0; i < size(); i++) {
        Node* n = (*this)[i];
        //~ debug("Updating parent of %s@%p, parent: %p -> %p%s", n->type, n, n->parent, this, n->parent == this ? " (NO CHANGE)" : "");
        n->parent = this;
        n->update_parents();
    }
}
