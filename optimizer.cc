#include "optimizer.h"
#include "config.h"
#include "log.h"

Optimizer::Optimizer(Grammar& g) : g(g) {}

int Optimizer::apply(const Optimization& config, const std::function<bool(Node&, int&)>& transform) {
    if (!Config::get(config)) {
        return 0;
    }
    int optimized = 0;
    g.map([optimized, transform](Node& node) mutable -> bool {
        return transform(node, optimized);
    });
    return optimized;
}

int Optimizer::concat_strings() {
    // "A" "B" -> "AB"
    return apply(O_CONCAT_STRINGS, [](Node& node, int& optimized) -> bool {
        Sequence* s = node.as<Sequence>();
        if (!s) return false;

        String* prev = nullptr;
        int prev_index;
        for (int i = s->terms.size() - 1; i >= 0; i--) {
            Term& t = s->terms[i];
            if (!t.quantifier && !t.prefix && std::holds_alternative<String>(t.primary)) {
                String* str = std::get_if<String>(&(t.primary));
                if (prev) {
                    log(1, "Merging adjacent strings: %s + %s", str->content.c_str(), prev->content.c_str());
                    str->content += prev->content;
                    s->terms.erase(s->terms.begin() + prev_index);
                    optimized++;
                }
                prev = str;
                prev_index = i;
            } else {
                prev = nullptr;
            }
        }
        return optimized;
    });
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

        for (int i = 1; i < s->terms.size(); i++) {
            Term& t1 = s->terms[i-1];
            Term& t2 = s->terms[i];
            if (t1.primary != t2.primary || t1.prefix || t2.prefix) continue;

            if (t1.is_greedy() && t2.is_optional()) {
                s->terms.erase(s->terms.begin()+i);
                s->update_parents();
                return true;
            } else if (t1.is_greedy() && !t2.is_optional()) {
                warn("Detected sequence that will never match: %s %s", t1.to_string().c_str(), t2.to_string().c_str());
                continue;
            } else if (t1.quantifier == '?') {
                switch (t2.quantifier) {
                case '*':
                    s->terms.erase(s->terms.begin() + i - 1);
                    s->update_parents();
                    return true;
                case '+':
                    t1.quantifier = 0;
                    return true;
                case 0:
                    t1.quantifier = 0;
                    t2.quantifier = '?';
                    return true;
                }
            } else if (t1.quantifier == 0 && t2.quantifier == '*') {
                t1.quantifier = '+';
                s->terms.erase(s->terms.begin() + i);
                s->update_parents();
                return true;
            }
        }
        return false;
    });
}

int Optimizer::normalize_character_classes() {
    return apply(O_NORMALIZE_CHAR_CLASS, [](Node& node, int& optimized) -> bool {
        CharacterClass* cc = node.as<CharacterClass>();
        if (!cc || cc->content == ".") return false;
        std::string orig = cc->content;
        cc->normalize();
        if (cc->content != orig) {
            optimized++;
        }
        return false; // this doesn't move any nodes, so we can always return false
    });
}

int Optimizer::single_char_character_classes() {
    // [A] -> "A"
    // [^B] -> !"B"
    return apply(O_SINGLE_CHAR_CLASS, [](Node& node, int& optimized) -> bool {
        CharacterClass* cc = node.as<CharacterClass>();
        if (!cc || cc->content == ".") return false;
        int size = cc->content.size() + (cc->dash ? 1 : 0);
        if (size != 1) return false;
        Term* parent = cc->get_parent<Term>();
        if (!parent) return false; // should never happen
        log(1, "Optimizing character class: %s", cc->to_string().c_str());
        if (cc->negation) {
            parent->prefix = parent->prefix == '!' ? 0 : '!';
        }
        parent->primary = Primary(String(cc->dash ? "-" : cc->content, parent));
        optimized++;
        return true;
    });
}

int Optimizer::character_class_negations() {
    // ![A] -> [^A]
    // ![^A] -> [A]
    return apply(O_CHAR_CLASS_NEGATION, [](Node& node, int& optimized) -> bool {
        Term* t = node.as<Term>();
        if (!t || !t->contains<CharacterClass>() || t->prefix != '!') return false;
        CharacterClass cc = t->get<CharacterClass>();
        if (cc.content == ".") return false;
        log(1, "Simplifying character class negation: %s", t->to_string().c_str());
        cc.negation = !cc.negation;
        t->prefix = 0;
        t->primary = cc;
        t->update_parents();
        return true;
    });
}

int Optimizer::double_negations() {
    // !(!A) -> A
    return apply(O_DOUBLE_NEGATION, [](Node& node, int& optimized) -> bool {
        Term* t = node.as<Term>();
        if (!t || !t->contains<Group>() || t->prefix != '!') return false;
        Group group = t->get<Group>();
        if (!group.has_single_term()) return false;
        Term inner_term = group.expression->sequences[0].terms[0];
        if (inner_term.prefix != '!') return false;
        log(1, "Optimizing double negation: %s", t->to_string().c_str());
        *t = inner_term;
        t->prefix = 0;
        t->update_parents();
        return true;
    });
}

int Optimizer::double_quantifications() {
    // _B__|_B*_|_B?_|_B+_
    //  A* | A* | A* | A*
    //  A? | A* | A? | A*
    //  A+ | A* | A* | A+   e.g.: (A?)? -> A?
    return apply(O_DOUBLE_QUANTIFICATION, [](Node& node, int& optimized) -> bool {
        Term* t = node.as<Term>();
        if (!t || !t->contains<Group>() || t->quantifier == 0) return false;
        Group group = t->get<Group>();
        if (!group.has_single_term()) return false;
        Term inner_term = group.expression->sequences[0].terms[0];
        if (inner_term.quantifier == 0 || inner_term.prefix) return false;
        log(1, "Optimizing double quantification: %s", t->to_string().c_str());
        int q = (inner_term.quantifier == t->quantifier) ? t->quantifier : '*';
        t->primary = inner_term.primary;
        t->quantifier = q;
        t->update_parents();
        return true;
    });
}

int Optimizer::remove_unnecessary_groups() {
    return apply(O_REMOVE_GROUP, [](Node& node, int& optimized) -> bool {
        Term* t = node.as<Term>();
        if (!t || !t->contains<Group>()) return false;
        Group group = t->get<Group>();
        if (group.capture) return false;
        if (group.expression->sequences.size() > 1) return false;

        if (!t->prefix && !t->quantifier) {
            // A (B C) D -> A B C D
            log(1, "Removing grouping from '%s'", group.parent->to_string().c_str());
            Sequence* s = t->parent->as<Sequence>();
            int pos;
            for (pos = 0; pos < s->size(); pos++) {
                if (&(s->terms[pos]) == t) break;
            }
            s->terms.erase(s->terms.begin()+pos);
            s->terms.insert(s->terms.begin()+pos, group.expression->sequences[0].terms.begin(), group.expression->sequences[0].terms.end());
            s->update_parents();
            optimized++;
        } else if (group.has_single_term() && !group.expression->sequences[0].terms[0].prefix && !group.expression->sequences[0].terms[0].quantifier) {
            // A (B)* C -> A B* C
            log(1, "Removing grouping from %s", group.parent->to_string().c_str());
            t->primary = group.expression->sequences[0].terms[0].primary;
            t->update_parents();
            optimized++;
        }
        return optimized;
    });
}

int Optimizer::unused_variables() {
    return apply(O_UNUSED_VARIABLE, [](Node& node, int& optimized) -> bool {
        Reference* r = node.as<Reference>();
        if (!r || r->var.empty()) return false;
        Rule* rule = node.get_ancestor<Rule>();
        std::vector<Action*> actions = rule->find_all<Action>([r](const Action& action) -> bool {
            return action.contains_var(r->var);
        });
        if (!actions.empty()) return false;
        log(1, "Removing unused variable reference '%s' in rule %s.", r->var.c_str(), rule->to_string().c_str());
        r->var.clear();
        return true;
    });
}

int Optimizer::inline_rules() {
    if (!Config::get(O_INLINE)) {
        return 0;
    }
    int optimized = 0;
    for (int i = g.rules.size() - 1; i >= 0; i--) {
        Rule& rule = g.rules[i];
        if (rule.expression.sequences.size() > 1) continue; // do not inline rules with alternation

        // check for direct recursion
        bool is_recursive = !rule.find_all<Reference>([rule](const Reference& node) -> bool {
            return node.name == rule.name;
        }).empty();
        if (is_recursive) continue;

        std::vector<Reference*> refs = g.find_all<Reference>([rule](const Reference& node) -> bool {
            Reference* ref = node.as<Reference>();
            if (!ref) return false;
            return ref->name == rule.name;
        });
        bool is_terminal = rule.is_terminal();
        int inline_limit = Config::get<int>(is_terminal ? "terminal-inline-limit" : "inline-limit");
        log(2, "Found %d references to rule %s, limit = %d", refs.size(), rule.name.c_str(), inline_limit);
        if (refs.size() == 0 /* main rule */ || refs.size() > inline_limit) continue;

        Term src = rule.expression.sequences[0].terms[0];
        log(1, "Inlining rule %s", rule.name.c_str());
        for (int j = 0; j < refs.size(); j++) {
            Term* dest = refs[j]->parent->as<Term>();
            Group group(*(src.parent->parent->as<Alternation>()), nullptr);
            log(2, "Inlining %s into %s", group.to_string().c_str(), dest->to_string().c_str());
            dest->primary = group;
            dest->update_parents();
            debug("Inlining result: %s", dest->to_string().c_str());
        }
        log(2, "Removing inlined rule %s", rule.name.c_str());
        g.rules.erase(g.rules.begin() + i);
        optimized++;
    }
    return optimized;
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
        opts += character_class_negations();
        opts += double_negations();
        opts += double_quantifications();
        opts += simplify_repeats();
        opts += concat_strings();
        opts += unused_variables();
        if (opts) debug("Grammar after pass %d (%d optimizations):\n%s", pass, opts, g.to_string().c_str());
        pass++;
    }
    return g;
}
