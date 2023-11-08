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

void Node::parse_comments(Parser& p, bool post) {
    debug("Parsing %s for node of type %s", post ? "post-comment" : "comments", type);
    if (post) {
        if (p.match_re("[ \t]*#([^\\n]*)", false)) {
            post_comment = p.last_re_match.str(1);
            debug("Post-comment: '%s'", post_comment.c_str());
        }
    } else {
        p.skip_space();
        while (p.match_comment()) {
            std::string comment = p.last_match;
            if (comment.back() == '\n') {
                comment.pop_back();
            }
            comments.push_back(comment);
            debug("Comment: '%s'", comment.c_str());
            if (p.peek_re("(\\s*\\n)+\\s*\\#", false)) {
                // it would be hard to keep the empty lines between comments,
                // so we just replace them with empty commented lines
                p.skip_space();
                debug("Detected empty line(s) between comments\n");
                comments.push_back("");
            }
        }
    }
}

std::string Node::format_comments(std::string indent) const {
    if (comments.empty()) return "";
    std::string result;
    for(int i = 0; i < comments.size(); i++) {
        result += (i ? "\n" : "") + indent + "#" + comments[i];
    }
    return result;
}

std::string Node::dump_comments() const {
    if (comments.empty() && post_comment.empty()) {
        return "";
    }
    std::string result = "";
    if (!comments.empty()) {
        result += std::to_string(comments.size()) + " comment" + (comments.size() > 1 ? "s" : "");
    }
    if (!post_comment.empty()) {
        result += result.empty() ? "post-comment" : ", post-comment";
    }
    return " (" + result + ")";
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
