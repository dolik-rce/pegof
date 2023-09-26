#include "ast/grammar.h"

class Optimizer {
    Grammar& g;

    int apply(const char* config, const std::function<bool(Node&, int&)>& transform);

    int inline_rules();
    int concat_strings();
    int normalize_character_classes();
    int single_char_character_classes();
    int character_class_negations();
    int double_negations();
    int double_quantifications();
    int remove_unnecessary_groups();
public:
    Optimizer(Grammar& g);
    Grammar optimize();
};
