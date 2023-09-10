#pragma once
#include <string>
#include <optional>
#include <regex>
#include <variant>
#include <sstream>
#include <iomanip>

#include "parser.h"

class Node {
protected:
    bool valid;
    Node(const char* type): valid(false), type(type) {}
public:
    const char* type;
    std::string comment;

    virtual void parse(Parser2& p) = 0;
    virtual std::string to_string() const = 0;
    virtual std::string dump(std::string indent = "") const = 0;

    operator bool();

    template<class U>
    bool is() const;

    virtual Node* operator[](int index);

    virtual long size() const;

    template <class U>
    std::vector<U*> find_all(const std::function<bool(const U&)>& predicate);

    template <class U>
    void find_all(std::vector<U*>& result, const std::function<bool(const U&)>& predicate);

    std::string to_c_string(std::string str) const;

    void parse_comments(Parser2& p);
    std::string comments(std::string indent = "") const;
};

template<class U>
bool Node::is() const {
    return typeid(*this) == typeid(U);
}

template <class U>
std::vector<U*> Node::find_all(const std::function<bool(const U&)>& predicate) {
    std::vector<U*> result;
    find_all(result, predicate);
    return result;
}

template <class U>
void Node::find_all(std::vector<U*>& result, const std::function<bool(const U&)>& predicate) {
    if (is<U>() && predicate(*(U*)this)) {
        result.push_back((U*)(this));
    }
    for (int i = 1; i < size(); i++) {
        Node* n = (*this)[i];
        n->find_all(result, predicate);
    }
}
