#pragma once
#include <string>
#include <optional>

#include "parser.h"

template<class T>
class Node {
protected:
    bool valid;
    Node(): valid(false) {}
public:
    operator bool() {
        return valid;
    }
    virtual void parse(Parser2& p) = 0;
    virtual std::string to_string() const = 0;
    //virtual std::string dump(int level) const = 0;
};
