#include "ast/grammar.h"

class Optimizer {
    Grammar& g;

    int inline_rules();
    int concat_strings();
    int normalize_character_classes();
    int single_char_character_classes();
    int character_class_negations();
    int double_negations();
    int remove_unnecessary_groups();
public:
    Optimizer(Grammar& g);
    Grammar optimize();
};
