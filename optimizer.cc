#include "optimizer.h"
#include "config.h"

Optimizer::Optimizer(Grammar& g) : g(g) {}

int Optimizer::concat_strings() {
    if (Config::get<bool>("no-concat")) {
        return 0;
    }
    int optimized = 0;
    g.map([optimized](Node& node) mutable {
        Sequence* s = node.as<Sequence>();
        if (!s) return;

        String* prev = nullptr;
        int prev_index;
        for (int i = s->terms.size() - 1; i >= 0; i--) {
            Term& t = s->terms[i];
            if (!t.quantifier && !t.prefix && std::holds_alternative<String>(t.primary)) {
                String* str = std::get_if<String>(&(t.primary));
                if (prev) {
                    //~ printf("Merging adjacent strings: %s + %s\n", str->content.c_str(), prev->content.c_str());
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
    });
    return optimized;
}

int Optimizer::normalize_character_classes() {
    if (Config::get<bool>("no-char-class")) {
        return 0;
    }
    int optimized = 0;
    g.map([optimized](Node& node) mutable {
        CharacterClass2* cc = node.as<CharacterClass2>();
        if (!cc || cc->content == ".") return;
        std::string orig = cc->content;
        cc->normalize();
        if (cc->content != orig) optimized++;
    });
    return optimized;
}

int Optimizer::single_char_character_classes() {
    if (Config::get<bool>("no-single-char")) {
        return 0;
    }
    int optimized = 0;
    g.map([optimized](Node& node) mutable {
        CharacterClass2* cc = node.as<CharacterClass2>();
        if (!cc || cc->content == ".") return;
        int size = cc->content.size() + (cc->dash ? 1 : 0);
        if (size != 1) return;
        Term* parent = cc->get_parent<Term>();
        if (!parent) return; // should never happen
        //~ printf("Optimizing character class: %s\n", cc->dump().c_str());
        if (cc->negation) {
            parent->prefix = parent->prefix == '!' ? 0 : '!';
        }
        parent->primary = Primary(String(cc->dash ? "-" : cc->content, parent));
    });
    return optimized;
}

int Optimizer::inline_rules() {
    int optimized = 0;
    for (int i = g.rules.size() - 1; i >= 0; i--) {
        Rule& rule = g.rules[i];
        if (rule.expression.sequences.size() > 1) continue; // do not inline rules with alternation

        // check for direct recursion
        bool is_recursive = !rule.find_all<Reference>([rule](const Reference& node) -> bool {
            return node.name == rule.name;
        }).empty();
        if (is_recursive) continue;

        std::vector<Reference*> refs = g.find_all<Reference>([rule, optimized](const Reference& node) -> bool {
            Reference* ref = node.as<Reference>();
            if (!ref) return false;
            return ref->name == rule.name;
        });
        bool is_terminal = rule.is_terminal();
        int inline_limit = Config::get<int>(is_terminal ? "terminal-inline-limit" : "inline-limit");
        //~ printf("DBG: found %d references to rule %s, limit = %d\n", refs.size(), rule.name.c_str(), inline_limit);
        if (refs.size() == 0 /* main rule */ || refs.size() > inline_limit) continue;

        Term src = rule.expression.sequences[0].terms[0];
        for (int j = 0; j < refs.size(); j++) {
            Term* dest = refs[j]->parent->as<Term>();
            Group group(*(src.parent->parent->as<Alternation>()), nullptr);
            //~ printf("DBG: inlining \n%s\nDBG:into \n%s\n", group.dump().c_str(), dest->dump().c_str());
            dest->primary = group;
            dest->update_parents();
            //~ printf("DBG: result: \n%s\n", dest->dump().c_str());
        }
        //~ printf("DBG: removing rule %s\n", rule.name.c_str());
        g.rules.erase(g.rules.begin() + i);
        optimized++;
    }
    return optimized;
}

Grammar Optimizer::optimize() {
    int opts = 1;
    while (opts > 0) {
        opts = normalize_character_classes();
        opts += inline_rules();
        opts += single_char_character_classes();
        opts += concat_strings();
    }
    return g;
}
