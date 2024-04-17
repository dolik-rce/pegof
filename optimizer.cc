#include "optimizer.h"
#include "config.h"
#include "utils.h"
#include "log.h"

#include <set>
#include <math.h>

Optimizer::Optimizer(Grammar& g) : g(g) {}

void Optimizer::warn_once(const std::string& warning) {
    static std::set<std::string> warnings;
    if (warnings.count(warning) == 0) {
        warn("%s", warning.c_str());
        warnings.insert(warning);
    }
}

int Optimizer::apply(const Optimization& config, const std::function<bool(Node&, int&)>& transform) {
    if (!Config::get(config)) {
        return 0;
    }
    int optimized = 0;
    g.map([&optimized, transform](Node& node) mutable -> bool {
        return transform(node, optimized);
    });
    return optimized;
}

int Optimizer::concat_strings() {
    // "A" "B" -> "AB"
    return apply(O_CONCAT_STRINGS, [](Node& node, int& optimized) -> bool {
        Sequence* s = node.as<Sequence>();
        if (!s) return false;

        String* prev_str = nullptr;
        Term* prev_term = nullptr;
        for (int i = s->size() - 1; i >= 0; i--) {
            Term& t = s->get(i);
            if (t.is_simple() && t.contains<String>()) {
                String& str = t.get<String>();
                if (prev_str) {
                    log(1, "Merging adjacent strings: %s + %s", str.c_str(), prev_str->c_str());
                    str.append(prev_str->c_str());
                    s->erase(prev_term);
                    optimized++;
                }
                prev_str = &str;
                prev_term = &t;
            } else {
                prev_str = nullptr;
                prev_term = nullptr;
            }
        }
        return optimized > 0;
    });
}

int Optimizer::concat_character_classes() {
    // [AB] / [CD] -> [ABCD]
    return apply(O_CONCAT_CHAR_CLASSES, [](Node& node, int& optimized) -> bool {
        Alternation* a = node.as<Alternation>();
        if (!a) return false;

        Term* prev_term = nullptr;
        for (int i = a->size() - 1; i >= 0; i--) {
            Sequence& s = a->get(i);
            Term& t = s.get_first_term();
            if (s.has_single_term() && t.contains<CharacterClass>()) {
                if (prev_term && prev_term->same_prefix(t) && prev_term->same_quantifier(t)) {
                    CharacterClass& cc1 = t.get<CharacterClass>();
                    CharacterClass& cc2 = prev_term->get<CharacterClass>();
                    if (cc1.is_negative() == cc2.is_negative() && !cc1.any_char() && !cc1.any_char()) {
                        log(1, "Merging character classes: %s + %s", t.to_string().c_str(), prev_term->to_string().c_str());
                        cc1.merge(cc2);
                        a->erase(i+1);
                        optimized++;
                    }
                }
                prev_term = &t;
            } else {
                prev_term = nullptr;
            }
        }
        return optimized > 0;
    });
}

int optimize_repeating_terms(Term& t1, Term& t2) {
    if (t1.primary != t2.primary || t1.prefix || t2.prefix)
        return -1; // do nothing
    Sequence* s = t1.parent->as<Sequence>();

    if (t1.is_greedy() && t2.is_optional()) {
        return 2; // delete t2
    } else if (t1.is_greedy() && !t2.is_optional()) {
        Optimizer::warn_once("Detected sequence that will never match: " + t1.to_string() + " " + t2.to_string());
        return -1; //do nothing
    } else if (t1.quantifier == '?') {
        switch (t2.quantifier) {
        case '*':
            return 1; // delete t1
        case '+':
            t1.quantifier = 0;
            return 0;
        case 0:
            t1.quantifier = 0;
            t2.quantifier = '?';
            return 0;
        }
    } else if (t1.quantifier == 0 && t2.quantifier == '*') {
        t1.quantifier = '+';
        return 2;  // delete t2
    }
    return -1;
}

int Optimizer::simplify_repeats() {
    // 1    2: *                  ?                  +                  ""
    // *       X* X* -> X*        X* X? -> X*        X* X+ -> WARN      X* X -> WARN
    // +       X+ X* -> X+        X+ X? -> X+        X+ X+ -> WARN      X+ X -> WARN
    // ?       X? X* -> X*        X? X? -> X? X?     X? X+ -> X X+      X? X -> X X?
    // ""      X  X* -> X+        X  X? -> X X?      X  X+ -> X X+      X  X -> X X
    return apply(O_REPEATS, [](Node& node, int& optimized) -> bool {
        Sequence* s = node.as<Sequence>();
        if (!s) return false;

        for (int i = 1; i < s->size(); i++) {
            switch (optimize_repeating_terms(s->get(i-1), s->get(i))) {
            case -1: continue;
            case 0: optimized++; return true;
            case 1: s->erase(i - 1); break;
            case 2: s->erase(i); break;
            }
            s->update_parents();
            optimized++;
            return true;
        }
        return false;
    });
}

int Optimizer::normalize_character_classes() {
    return apply(O_NORMALIZE_CHAR_CLASS, [](Node& node, int& optimized) -> bool {
        CharacterClass* cc = node.as<CharacterClass>();
        if (!cc || cc->any_char()) return false;
        if (cc->normalize()) {
            optimized++;
        }
        return false; // this doesn't move any nodes, so we can always return false
    });
}

int Optimizer::single_char_character_classes() {
    // [A] -> "A"
    return apply(O_SINGLE_CHAR_CLASS, [](Node& node, int& optimized) -> bool {
        CharacterClass* cc = node.as<CharacterClass>();
        if (!cc || cc->any_char() || cc->is_negative() || !cc->is_single_char()) return false;
        Term* parent = cc->get_parent<Term>();
        if (!parent) return false; // should never happen
        log(1, "Optimizing character class: %s", cc->to_string().c_str());
        parent->set_content(cc->convert_to_string());
        optimized++;
        return true;
    });
}

// THIS IS WRONG: '[^xy]' is equal to '![xy] .', not just ![xy]
//~ int Optimizer::character_class_negations() {
//~     // ![A] -> [^A]
//~     // ![^A] -> [A]
//~     return apply(O_CHAR_CLASS_NEGATION, [](Node& node, int& optimized) -> bool {
//~         Term* t = node.as<Term>();
//~         if (!t || !t->contains<CharacterClass>() || !t->is_negative()) return false;
//~         CharacterClass cc = t->get<CharacterClass>();
//~         if (cc.any_char()) return false;
//~         log(1, "Simplifying character class negation: %s", t->to_string().c_str());
//~         cc.flip_negation();
//~         t->set_prefix(0);
//~         t->set_content(cc);
//~         t->update_parents();
//~         optimized++;
//~         return true;
//~     });
//~ }

int Optimizer::double_negations() {
    // !(!A) -> A
    return apply(O_DOUBLE_NEGATION, [](Node& node, int& optimized) -> bool {
        Term* t = node.as<Term>();
        if (!t || !t->contains<Group>() || !t->is_negative()) return false;
        Group group = t->get<Group>();
        if (!group.has_single_term()) return false;
        Term inner_term = group.get_first_term();
        if (!inner_term.is_negative()) return false;
        log(1, "Optimizing double negation: %s", t->to_string().c_str());
        *t = inner_term;
        t->set_prefix(0);
        t->update_parents();
        optimized++;
        return true;
    });
}

int optimize_double_quantifiers(const Term& outer, const Term& inner) {
    return (inner.quantifier == outer.quantifier) ? outer.quantifier : '*';
}

int Optimizer::double_quantifications() {
    // _B__|_B*_|_B?_|_B+_
    //  A* | A* | A* | A*
    //  A? | A* | A? | A*
    //  A+ | A* | A* | A+   e.g.: (A?)? -> A?
    return apply(O_DOUBLE_QUANTIFICATION, [](Node& node, int& optimized) -> bool {
        Term* t = node.as<Term>();
        if (!t || !t->contains<Group>() || !t->is_quantified()) return false;
        Group group = t->get<Group>();
        if (!group.has_single_term()) return false;
        Term inner_term = group.get_first_term();
        if (!inner_term.is_quantified() || inner_term.is_prefixed()) return false;
        log(1, "Optimizing double quantification: %s", t->to_string().c_str());
        t->copy_content(inner_term);
        t->set_quantifier(optimize_double_quantifiers(*t, inner_term));
        t->update_parents();
        optimized++;
        return true;
    });
}

int Optimizer::remove_unnecessary_groups() {
    return apply(O_REMOVE_GROUP, [](Node& node, int& optimized) -> bool {
        Alternation* a = node.as<Alternation>();
        if (a) {
            for (int pos = 0; pos < a->size(); pos++) {
                Sequence& s = a->get(pos);
                if (!s.has_single_term()) continue;
                Term term = s.get_first_term();
                if (!term.is_simple() || !term.contains<Group>()) continue;
                // A / (B / C) / D -> A / B / C / D
                log(1, "Removing grouping from '%s'", term.to_string().c_str());
                Group group = term.get<Group>();
                a->erase(pos);
                a->insert(pos, group.convert_to_alternation());
                a->update_parents();
                optimized++;
                return true;
            }
        }
        Term* t = node.as<Term>();
        if (!t || !t->contains<Group>()) return false;
        Group group = t->get<Group>();
        if (!group.has_single_sequence()) return false;
        const Term& first_term = group.get_first_term();
        if (t->is_simple()) {
            // A (B C) D -> A B C D
            log(1, "Removing grouping from '%s'", t->to_string().c_str());
            Sequence* s = t->parent->as<Sequence>();
            int pos;
            for (pos = 0; pos < s->size(); pos++) {
                if (&(s->get(pos)) == t) break;
            }
            s->erase(pos);
            s->insert(pos, group.get_first_sequence());
            s->update_parents();
            optimized++;
            return true;
        } else if (group.has_single_term() && first_term.is_simple()) {
            // A (B)* C -> A B* C
            log(1, "Removing grouping from %s", t->to_string().c_str());
            t->copy_content(first_term);
            t->update_parents();
            optimized++;
            return true;
        }
        return false;
    });
}

int Optimizer::unused_variables() {
    return apply(O_UNUSED_VARIABLE, [](Node& node, int& optimized) -> bool {
        Reference* r = node.as<Reference>();
        if (!r || !r->has_variable()) return false;
        Rule* rule = node.get_ancestor<Rule>();
        std::vector<Action*> actions = rule->find_all<Action>([r](const Action& action) -> bool {
            return action.contains_reference(*r);
        });
        if (!actions.empty()) return false;
        log(1, "Removing unused variable reference from '%s' in rule %s.", r->to_string().c_str(), rule->to_string().c_str());
        r->remove_variable();
        optimized++;
        return true;
    });
}

int Optimizer::unused_captures() {
    return apply(O_UNUSED_CAPTURE, [](Node& node, int& optimized) -> bool {
        Rule* rule = node.as<Rule>();
        if (!rule) return false;
        std::vector<Capture*> captures = rule->find_all<Capture>();
        if (captures.empty()) return false;
        std::vector<Expand*> expands = rule->find_all<Expand>();
        std::vector<Action*> actions = rule->find_all<Action>();

        for (int i = 0; i < captures.size(); i++) {
            bool used_in_source = std::any_of(actions.begin(), actions.end(), [i](const Action* action){
                return action->contains_capture(i + 1);
            });
            bool used_in_expand = std::any_of(expands.begin(), expands.end(), [i](const Expand* expand){
                return *expand == i + 1;
            });
            if (used_in_source || used_in_expand) {
                continue;
            }
            log(1, "Removing unused capture '%s' in rule %s.", captures[i]->to_string().c_str(), rule->to_string().c_str());
            ;
            Term* parent = captures[i]->get_parent<Term>();
            parent->set_content(captures[i]->convert_to_group());
            parent->update_parents();
            for (int j = 0; j < expands.size(); j++) {
                if (*expands[j] <= i) continue;
                std::string prev = expands[j]->to_string();
                expands[j]->shift(-1);
                log(2, "Replacing expand '%d' -> '%d'", prev.c_str(), expands[j]->to_string().c_str());
            }
            for (int j = 0; j < actions.size(); j++) {
                for (int k = i+2; k <= captures.size(); k++) {
                    log(2, "Replacing '$%d' -> '$%d' in action %s", k, k-1, actions[j]->to_string().c_str());
                    actions[j]->renumber_capture(k, k - 1);
                }
            }
            optimized++;
            return true;
        }
        return false;
    });
}

static double calculate_score(int term_count, int ref_count) {
    if (term_count == 1) return 1;
    if (ref_count <= 1) return 1;
    return 1.0 / sqrt(term_count * ref_count);
}

int Optimizer::inline_rules() {
    if (!Config::get(O_INLINE)) {
        return 0;
    }
    double best_score = 0;
    int candidate = -1;
    int optimized = 0;
    double min_score = Config::get<double>("inline-limit");

    std::vector<Rule*> rules = g.find_all<Rule>();
    // intentionally skipping the first rule, because it is the main one, which can't be inlined anyway
    for (int i = rules.size() - 1; i > 0; i--) {
        Rule& rule = *rules[i];

        // check for direct recursion
        bool is_recursive = !rule.find_all<Reference>([rule](const Reference& ref) -> bool {
            return ref.references(&rule);
        }).empty();
        if (is_recursive) {
            log(2, "Not inlining %s: rule is recursive", rule.c_str());
            continue;
        }

        std::vector<Reference*> refs = g.find_all<Reference>([rule](const Reference& ref) -> bool {
            return ref.references(&rule);
        });

        if (std::any_of(refs.begin(), refs.end(), [](Reference* r){
            return r->has_variable();
        })) {
            log(2, "Not inlining %s: rule is used with variables", rule.c_str());
            continue;
        }

        double score = calculate_score(rule.count_terms() + rule.count_cc_tokens(), refs.size());
        log(4, "Score for %s: %f", rule.c_str(), score);

        if (score > best_score) {
            best_score = score;
            candidate = i;
        }
    }
    if (candidate >= 0 && best_score >= min_score) {
        Rule& rule = *rules[candidate];
        std::vector<Reference*> refs = g.find_all<Reference>([rule](const Reference& ref) -> bool {
            return ref.references(&rule);
        });

        int src_captures = rule.find_all<Capture>().size();

        log(1, "Inlining rule %s (score %f)", rule.c_str(), best_score);
        for (int j = 0; j < refs.size(); j++) {
            Term* dest = refs[j]->parent->as<Term>();
            Group group = rule.convert_to_group();
            log(2, "  Inlining %s into %s", group.to_string().c_str(), dest->to_string().c_str());
            dest->set_content(group);
            dest->update_parents();
            // fix capture references in expands and actions
            if (src_captures) {
                Rule* dest_rule = dest->get_ancestor<Rule>();
                int dest_captures = dest_rule->find_all<Capture>().size();
                if (dest_captures) {
                    // Algorithm:
                    //   shift = how many captures is before the insertion point
                    //   src_captures = how many captures is in the inserted group
                    //  - first iterate over rule,
                    //     - count captures before group (N)
                    //     - skip the inserted group
                    //     - increment expands and actions after the group by M
                    //   - then iterate over the group only
                    //     - increase expands and actions by before number found in first iteration
                    // example: input:   <1> <2> (<1> <2> <3>) <3> <4>
                    //          shift:            +2  +2  +2   +3  +3
                    //          output:  <1> <2> (<3> <4> <5>) <6> <7>
                    dest_rule->update_captures();
                    bool after = false;
                    int shift = 0;
                    dest_rule->map([&](Node& node) mutable {
                        if (node.is_descendant_of(dest)) {
                            after = true;
                            return false;
                        }
                        if (!after && node.is<Capture>()) {
                            shift++;
                            return false;
                        }
                        if (after && node.is<Expand>()) {
                            std::string prev = node.to_string();
                            Expand *e = node.as<Expand>();
                            e->shift(src_captures);
                            log(2, "  Update expand: %s -> %s", prev.c_str(), node.to_string().c_str());
                        } else if (after && node.is<Action>()) {
                            std::string prev = node.to_string();
                            Action *a = node.as<Action>();
                            for (int k = dest_captures; k >= 1; k--) {
                                a->renumber_capture(k, k + src_captures);
                            }
                            log(2, "  Update action: %s -> %s", prev.c_str(), node.to_string().c_str());
                        }
                        return false;
                    });
                    dest->get<Group>().map([&](Node& node){
                        if (node.is<Expand>()) {
                            std::string prev = node.to_string();
                            Expand *e = node.as<Expand>();
                            e->shift(shift);
                            log(2, "  Update expand: %s -> %s", prev.c_str(), node.to_string().c_str());
                        } else if (node.is<Action>()) {
                            std::string prev = node.to_string();
                            Action *a = node.as<Action>();
                            for (int k = src_captures; k >= 1; k--) {
                                a->renumber_capture(k, k + shift);
                            }
                            log(2, "  Update action: %s -> %s", prev.c_str(), node.to_string().c_str());
                        }
                        return false;
                    });
                }
            }
            debug("  Inlining result: %s", dest->to_string().c_str());
        }
        log(2, "  Removing inlined rule %s", rule.c_str());
        g.erase(&rule);
        g.update_parents();
        optimized++;
        return true;
    }
    return false;
}

Grammar Optimizer::optimize() {
    int opts = 1;
    int pass = 1;
    debug("Input grammar:\n%s", g.to_string().c_str());
    while (opts > 0) {
        log(2, "Optimization pass %d", pass);
        opts = normalize_character_classes();
        opts += inline_rules();
        opts += remove_unnecessary_groups();
        opts += single_char_character_classes();
        //~ opts += character_class_negations();
        opts += double_negations();
        opts += double_quantifications();
        opts += simplify_repeats();
        opts += concat_strings();
        opts += concat_character_classes();
        opts += unused_variables();
        opts += unused_captures();
        if (opts) debug("Grammar after pass %d (%d optimizations):\n%s", pass, opts, g.to_string().c_str());
        pass++;
    }
    return g;
}
