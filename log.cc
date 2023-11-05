#include "log.h"

DebugIndent::DebugIndent() {
    DebugIndent::inc();
}

DebugIndent::~DebugIndent() {
    DebugIndent::dec();
};

int DebugIndent::indent(int increment) {
    static int indent = 0;
    indent += increment;
    return indent;
}

void DebugIndent::inc() {
    indent(1);
}

void DebugIndent::dec() {
    indent(-1);
}

void DebugIndent::print() {
    fprintf(stderr, "%*s", indent() * 2, "");
}
