#pragma once
#include "ast/grammar.h"
#include "config.h"

class Optimizer {
    Grammar& g;

    int apply(const Optimization& config, const std::function<bool(Node&, int&)>& transform);

    int inline_rules();
    int concat_strings();
    int concat_character_classes();
    int normalize_character_classes();
    int single_char_character_classes();
    int character_class_negations();
    int double_negations();
    int double_quantifications();
    int simplify_repeats();
    int remove_unnecessary_groups();
    int unused_variables();
    int unused_captures();
    int empty_actions();
public:
    static void warn_once(const std::string& warning);

    Optimizer(Grammar& g);
    Grammar optimize();
};
