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

Grammar Optimizer::optimize() {
    int opts = 1;
    while (opts > 0) {
        opts = normalize_character_classes();
        opts += single_char_character_classes();
        opts += concat_strings();
    }
    return g;
}
