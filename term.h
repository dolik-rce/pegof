#pragma once
#include <variant>

#include "node.h"
#include "stringnode.h"
#include "reference.h"
#include "character_class.h"
#include "expand.h"
#include "action.h"
#include "group.h"

using Primary = std::variant<std::monostate, String, Reference, CharacterClass, Expand, Action, Group>;

class Term : public Node {
public:
    char prefix;
    char quantifier;
    Primary primary;

    Term(char prefix, char quantifier, const Primary& primary, Node* parent);
    Term(Parser& p, Node* parent);

    template<class T>
    bool parse(Parser& p);

    virtual void parse(Parser& p);

    std::string to_string(const Primary& x) const;
    virtual std::string to_string() const override;
    std::string dump(const Primary& x, std::string indent) const;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;

    template<class T>
    bool contains() const;

    template<class T>
    T get() const;
};


template<class T>
bool Term::contains() const {
    return std::holds_alternative<T>(primary);
}

template<class T>
T Term::get() const {
    return std::get<T>(primary);
}
