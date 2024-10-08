#pragma once
#include <string>
#include <optional>
#include <regex>
#include <variant>

#include "parser.h"

class Node {
protected:
    bool valid;
    Node* parent;
    Node(const char* type, Node* parent);
    virtual ~Node();

    const char* type;
    std::vector<std::string> comments;
    std::string post_comment;

public:
    virtual void parse(Parser& p) = 0;
    virtual std::string to_string(std::string indent = "") const = 0;
    virtual std::string dump(std::string indent = "") const = 0;
    virtual bool is_multiline() const = 0;

    operator bool() const;

    template<class U>
    bool is() const;

    template<class U>
    U* as() const;

    template<class U>
    U* get_parent() const;

    template <class U>
    U* get_ancestor() const;

    bool is_descendant_of(Node* n) const;

    virtual Node* operator[](int index);

    virtual long size() const;

    void update_parents();

    template <class U>
    std::vector<U*> find_children(const std::function<bool(const U&)>& predicate=[] (const U & node)->bool { return true; });

    template <class U>
    void find_children(std::vector<U*>& result, const std::function<bool(const U&)>& predicate);

    template <class U>
    std::vector<U*> find_ancestors(const std::function<bool(const U&)>& predicate=[] (const U & node)->bool { return true; });

    template <class U>
    void find_ancestors(std::vector<U*>& result, const std::function<bool(const U&)>& predicate);

    bool map(const std::function<bool(Node&)>& transform);

    void parse_comments(Parser& p, bool store = true);
    void parse_post_comment(Parser& p);
    std::string format_comments(std::string indent = "") const;
    std::string dump_comments() const;

    bool has_comments() const;
    bool has_post_comment() const;

    friend bool operator==(const Node& a, const Node& b);
};

template<class U>
bool Node::is() const {
    return typeid(*this) == typeid(U);
}

template<class U>
U* Node::as() const {
    if (!is<U>()) return nullptr;
    return (U*)this;
}

template<class U>
U* Node::get_parent() const {
    if (!parent) return nullptr;
    return parent->as<U>();
}

template<class U>
U* Node::get_ancestor() const {
    if (!parent) return nullptr;
    if (parent->is<U>()) {
        return parent->as<U>();
    } else {
        return parent->get_ancestor<U>();
    }
}

template <class U>
std::vector<U*> Node::find_children(const std::function<bool(const U&)>& predicate) {
    std::vector<U*> result;
    find_children(result, predicate);
    return result;
}

template <class U>
void Node::find_children(std::vector<U*>& result, const std::function<bool(const U&)>& predicate) {
    if (is<U>() && predicate(*(U*)this)) {
        result.push_back((U*)(this));
    }
    for (int i = 0; i < size(); i++) {
        Node* n = (*this)[i];
        n->find_children(result, predicate);
    }
}

template <class U>
std::vector<U*> Node::find_ancestors(const std::function<bool(const U&)>& predicate) {
    std::vector<U*> result;
    find_ancestors(result, predicate);
    return result;
}

template <class U>
void Node::find_ancestors(std::vector<U*>& result, const std::function<bool(const U&)>& predicate) {
    if (is<U>() && predicate(*(U*)this)) {
        result.push_back((U*)(this));
    }
    if (parent) {
        parent->find_ancestors(result, predicate);
    }
}

bool operator==(const Node& a, const Node& b);
bool operator!=(const Node& a, const Node& b);

inline bool is_multiline(const Node& n) {
    return n.is_multiline();
}
