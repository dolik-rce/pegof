#pragma once
#include <variant>

#include "ast/node.h"
#include "ast/string.h"
#include "ast/reference.h"
#include "ast/character_class.h"
#include "ast/expand.h"
#include "ast/action.h"
#include "ast/capture.h"
#include "ast/group.h"

using Primary = std::variant<std::monostate, String, Reference, CharacterClass, Expand, Action, Group, Capture>;

class Term : public Node {
    char prefix;
    char quantifier;
    Primary primary;
public:
    Term(char prefix, char quantifier, const Primary& primary, Node* parent);
    Term(Parser& p, Node* parent);

    template<class T>
    bool parse(Parser& p);

    virtual void parse(Parser& p);

    std::string to_string(const Primary& x, const std::string& indent) const;
    virtual std::string to_string(std::string indent = "") const override;
    std::string dump(const Primary& x, std::string indent) const;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;

    template<class T>
    bool contains() const;

    template<class T>
    T& get();

    bool is_quantified() const;
    bool is_prefixed() const;
    bool is_greedy() const;
    bool is_optional() const;
    bool is_simple() const;
    bool is_negative() const;

    bool same_prefix(const Term& t);
    bool same_quantifier(const Term& t);

    void flip_negation();
    void set_prefix(int new_prefix);
    void set_quantifier(int new_quantifier);
    void set_content(Primary content);
    void copy_prefix(const Term& other);
    void copy_quantifier(const Term& other);
    void copy_content(const Term& other);

    friend bool operator==(const Term& a, const Term& b);
    friend int optimize_repeating_terms(Term& t1, Term& t2);
    friend int optimize_double_quantifiers(const Term& outer, const Term& inner);
};

bool operator==(const Term& a, const Term& b);
bool operator!=(const Term& a, const Term& b);

template<class T>
bool Term::contains() const {
    return std::holds_alternative<T>(primary);
}

template<class T>
T& Term::get() {
    return std::get<T>(primary);
}
