#include "ast.h"
#include "char_class.h"
#include "config.h"
#include "io.h"

#include <algorithm>
#include <regex>

int AstNode::optimize_strings() {
    if (Config::get<bool>("no-concat")) {
        return 0;
    }
    int optimized = 0;
    for (size_t i = 1; i < children.size(); i++) {
        AstNode* first = children[i-1];
        AstNode* second = children[i];
        if (first->type == AST_STRING && second->type == AST_STRING) {
            Io::debug("  Concatenating adjacent strings '%s' and '%s'\n", first->text.c_str(), second->text.c_str());
            first->text += second->text;
            first->line = -2;
            first->column = -2;
            delete second;
            children.erase(children.begin() + i);
            optimized++;
            i = 1; // try again from start
        }
    }
    return optimized;
}

int AstNode::optimize_character_class() {
    CharacterClass cc = CharacterClass(text).normalize();

    if (cc.size() == 1 && !Config::get<bool>("no-single-char")) {
        if (cc.negative()) {
            append_child(new AstNode(AST_PREFIX_OP, "!"));
            append_child(new AstNode(AST_STRING, cc.to_string().substr(1)));
            type = AST_PRIMARY;
            text = "";
        } else {
            type = AST_STRING;
            text = cc.to_string();
        }
        return 1;
    }

    if (Config::get<bool>("no-char-class")) {
        return 0;
    }
    std::string normalized = cc.to_string();
    if (normalized == text) {
        return 0;
    }
    text = normalized;
    return 1;
}

int AstNode::optimize_repeats() {
    if (Config::get<bool>("keep-repeats")) {
        return 0;
    }

    if (children.size() != 2 || children[1]->type != AST_POSTFIX_OP) {
        return 0;
    }

    AstNode *first(NULL), *second(NULL);
    char first_op, second_op;

    AstNode* sibling = find_prev_sibling();
    if (sibling) {
        AstNode* content;
        if (sibling->type == AST_PRIMARY
            && sibling->children.size() == 2
            && sibling->children[1]->type == AST_POSTFIX_OP
        ) {
            content = sibling->children[0];
            first_op = sibling->children[1]->text[0];
        } else {
            content = sibling;
            first_op = ' ';
        }
        if (*content == *children[0]) {
            first = sibling;
            second = this;
            second_op = children[1]->text[0];
        }
    }

    if (!first) {
        sibling = find_next_sibling();
        if (sibling) {
            AstNode* content;
            if (sibling->type == AST_PRIMARY
                && sibling->children.size() == 2
                && sibling->children[1]->type == AST_POSTFIX_OP
            ) {
                content = sibling->children[0];
                second_op = sibling->children[1]->text[0];
            } else {
                content = sibling;
                second_op = ' ';
            }
            if (*content == *children[0]) {
                first = this;
                first_op = children[1]->text[0];
                second = sibling;
            }
        }
    }

    if (!first) {
        return 0;
    }

    if (first_op == '*' && second_op == '*') {
        Io::debug("%s\n", "  Removing unnecessary repeated node (X* X* -> X*)");
        second->parent->remove_child(second);
    } else if (first_op == '*' || second_op == '*') {
        Io::debug("  Removing unnecessary repeated node (X%c X%c -> X+)\n", first_op, second_op);
        if (first->type == AST_PRIMARY) {
            first->children[1]->text[0] = '+';
            first->column = -1;
            first->line = -1;
            second->parent->remove_child(second);
        } else {
            second->children[1]->text[0] = '+';
            second->column = -1;
            second->line = -1;
            first->parent->remove_child(first);
        }
    } else {
        return 0;
    }
    return 1;
}

int AstNode::optimize_single_child() {
    if (children.size() != 1) {
        return 0;
    }
    Io::debug("  Removing unnecessary node of type %s\n", get_type_name());
    AstNode* child = children[0];
    text = child->text;
    type = child->type;
    line = child->line;
    column = child->column;
    children = child->children;
    child->children.clear(); // clear children to avoid their recursive deletion
    delete child;
    return 1;
}

int AstNode::optimize_children() {
    int optimized = 0;
    for (size_t i = 0; i < children.size(); i++) {
        optimized += children[i]->optimize();
    }
    return optimized;
}

int AstNode::optimize_strip_comment() {
    Io::debug("%s\n", "  Removing comment");
    parent->remove_child(this);
    return 1;
}

int AstNode::optimize_unused_captures() {
    if (Config::get<bool>("keep-captures")) {
        return 0;
    }
    std::vector<AstNode *> captures = find_all([](const AstNode& node) {
        return node.type == AST_CAPTURE;
    });
    if (captures.empty()) {
        return 0;
    }
    std::vector<AstNode*> sources = find_all([](const AstNode& node) {
        return node.type == AST_SOURCE;
    });
    std::vector<AstNode*> backrefs = find_all([](const AstNode& node) {
        return node.type == AST_BACKREF;
    });

    for (int i = 0; i < captures.size(); i++) {
        std::string ref("$" + std::to_string(i+1));
        std::regex re(".*\\" + ref + "\\b.*");
        bool used_in_source = std::any_of(sources.begin(), sources.end(), [re](const AstNode *node){
            return std::regex_match(node->text, re);
        });
        bool used_in_backref = std::any_of(backrefs.begin(), backrefs.end(), [ref](const AstNode *node){
            return ref == node->text;
        });
        if (used_in_source || used_in_backref) {
            continue;
        }
        Io::debug("  Removing unused capture $%d in rule '%s'\n", i+1, children[0]->text.c_str());

        // move following captures one index back
        for (int j = i + 1; j < captures.size(); j++) {
            std::regex re_capture("\\$" + std::to_string(j+1) + "\\b");
            std::string replace = "$$" + std::to_string(j);
            for (int s = 0; s < sources.size(); s++) {
                sources[s]->text = std::regex_replace(sources[s]->text, re_capture, replace);
            }
            for (int b = 0; b < backrefs.size(); b++) {
                if (backrefs[b]->text == "$" + std::to_string(j+1)) {
                    backrefs[b]->text = "$" + std::to_string(j);
                }
            }
        }
        // remove capture from AST
        AstNode* child = captures[i]->children[0];
        captures[i]->text = child->text;
        captures[i]->type = child->type;
        captures[i]->line = child->line;
        captures[i]->column = child->column;
        captures[i]->children = child->children;
        child->children.clear(); // clear children to avoid their recursive deletion
        delete child;
        return 1;
    }
    return 0;
}

int AstNode::optimize_unused_variable() {
    if (Config::get<bool>("keep-variables")) {
        return 0;
    }
    AstNode* rule = find_parent(AST_RULE);
    std::regex re(".*\\b" + text + "\\b.*");
    std::vector<AstNode*> sources = rule->find_all([re](const AstNode& node) {
        return node.type == AST_SOURCE && std::regex_match(node.text, re);
    });
    if (!sources.empty()) {
        return 0;
    }
    Io::debug("  Removing unused variable reference '%s' in rule '%s'\n", text.c_str(), rule->children[0]->text.c_str());
    parent->remove_child(this);
    return 1;
}

int AstNode::optimize_inline_rule() {
    // check that rule contents are inlinable
    AstNode* rule = children[1];
    std::vector<AstNode*> uninlinable = rule->find_all([this](const AstNode& node) {
        return node.type == AST_SOURCE
               || node.type == AST_VAR
               || node.type == AST_CAPTURE
               || (node.type == AST_REFNAME && node.text == children[0]->text); // contains direct recursion
    });
    if (!uninlinable.empty()) {
        return 0;
    }

    std::string name = children[0]->text;
    std::vector<AstNode*> refs = find_all([name](const AstNode& node) {
        return node.type == AST_REFNAME && node.text == name;
    }, true);

    bool is_terminal = rule->is_terminal();
    int inline_limit = Config::get<int>(is_terminal ? "terminal-inline-limit" : "inline-limit");
    if (refs.empty() || refs.size() > inline_limit) {
        return 0;
    }
    Io::debug(
        "  Inlining %srule '%s' at %ld site%s\n",
        is_terminal ? "terminal " : "", name.c_str(), refs.size(), refs.size() > 1 ? "s" : ""
    );

    // copy data to each reference
    for (int i = 0; i < refs.size(); i++) {
        refs[i]->text = rule->text;
        refs[i]->type = rule->type;
        refs[i]->line = rule->line;
        refs[i]->column = rule->column;
        refs[i]->children.clear();
        for (int j = 0; j < rule->children.size(); j++) {
            refs[i]->children.push_back(new AstNode(*rule->children[j], refs[i]));
        }
        if (refs[i]->parent->type == AST_PRIMARY) {
            // references with prefix/postfix need to be wrapped in parentheses, e.g. (A B)*
            AstNode* group = new AstNode(AST_GROUP);
            group->children.push_back(refs[i]);
            refs[i]->parent->replace_child(refs[i], group, false);
        }
    }
    // delete the rule
    parent->remove_child(this);
    return refs.size();
}

int AstNode::optimize_negation() {
    if (text != "!" || Config::get<bool>("no-negation")) {
        return 0;
    }
    AstNode* next = find_next_sibling();

    if (next != nullptr && next->type == AST_CHARCLASS) {
        type = AST_CHARCLASS;
        text = CharacterClass(next->text).toggle_negation().to_string();
        parent->remove_child(next);
        return 1;
    }

    if (next != nullptr && next->type == AST_GROUP && next->children.size() == 1) {
        AstNode* child = next->children[0];
        if (child->type == AST_PRIMARY && child->children.size() == 2) {
            AstNode* grandchild = child->children[0];
            if (grandchild->type == AST_PREFIX_OP && grandchild->text == "!") {
                parent->replace_child(this, new AstNode(*child->children[1], parent));
                parent->remove_child(next);
                return 1;
            }
        }
    }

    return 0;
}

int AstNode::optimize_grammar() {
    int total = 0;
    for (int optimized = -1, i = 1; optimized != 0; i++) {
        Io::debug("Optimization pass %d:\n", i);
        optimized = optimize_children();
        if (Config::get<bool>("verbose")) {
            stats("  Optimized grammar contains ");
        }
        if (optimized) {
            total += optimized;
            Io::debug(" => %d optimization%s done in pass %d (%d total)\n", optimized, optimized == 1 ? "" : "s", i, total);
        } else {
            Io::debug(" => No more optimizations found, ending now (total %d optimizations) \n", total);
        }
    };
    return total;
}

int AstNode::optimize() {
    switch (type) {
    case AST_GRAMMAR:           return optimize_grammar();
    case AST_RULE:              return optimize_children() + optimize_unused_captures() + optimize_inline_rule();
    case AST_ALTERNATION:       return optimize_children() + optimize_single_child();
    case AST_PRIMARY:           return optimize_children() + optimize_single_child() + optimize_repeats();
    case AST_RULEREF:           return optimize_children() + optimize_single_child();
    case AST_SEQUENCE:          return optimize_children() + optimize_single_child() + optimize_strings();
    case AST_COMMENT:           return optimize_strip_comment();
    case AST_VAR:               return optimize_unused_variable();
    case AST_CHARCLASS:         return optimize_character_class();
    case AST_PREFIX_OP:         return optimize_children() + optimize_negation();

    // everything else just call optimize on children
    case AST_DIRECTIVE:
    case AST_CODE:
    case AST_SOURCE:
    case AST_ERROR:
    case AST_STRING:
    case AST_REFNAME:
    case AST_GROUP:
    case AST_CAPTURE:
    case AST_RULE_NAME:
    case AST_DIRECTIVE_NAME:
    case AST_POSTFIX_OP:
    case AST_DOT:
    case AST_BACKREF:
        return optimize_children();
    }
    Io::debug("%s\n", "ERROR: unexpected AST node type!");
    exit(2);
}
