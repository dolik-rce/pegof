#include "grammar.h"

class Optimizer {
    Grammar g;

    int concat_strings();
public:
    Optimizer(const Grammar& g);
    Grammar optimize();
};
