#include "optimizer.h"
#include "config.h"

Optimizer::Optimizer(const Grammar& g) : g(g) {}

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

Grammar Optimizer::optimize() {
    int opts = 1;
    while (opts > 0) {
        opts = concat_strings();
    }
    return g;
}
