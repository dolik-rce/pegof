#include "ast.h"
#include "config.h"
#include "io.h"

#include <algorithm>
#include <climits>
#include <regex>
#include <sstream>

enum TrimType {
    TRIM_LEFT = 1,
    TRIM_RIGHT = 2,
    TRIM_BOTH = TRIM_LEFT | TRIM_RIGHT
};

static std::string trim(const std::string& str, TrimType type = TRIM_BOTH) {
    size_t start = (type & TRIM_LEFT) ? str.find_first_not_of(" \t\r\n") : 0;
    size_t end = (type & TRIM_RIGHT) ? str.find_last_not_of(" \t\r\n") + 1 : str.size();
    if (start == std::string::npos) {
        start = 0;
    }
    return std::string(str.c_str() + start, end - start);
}

static void reindent(const std::string& str, int baseindent) {
    std::stringstream stream(str);
    std::string line;
    std::vector<std::string> lines;
    int min_indent = INT_MAX;
    while(std::getline(stream, line)) {
        if (trim(line).empty()) {
            //FIXME: only skip empty lines for first or last lines
            continue;
        }
        size_t indent = line.find_first_not_of(" \t");
        if (indent < min_indent) {
            min_indent = indent;
        }
        lines.push_back(line);
    }
    for(int i = 0; i < lines.size(); i++) {
        int indent = std::max(min_indent - baseindent, 0);
        Io::print("%*s%s\n", baseindent + indent, "", trim(lines[i].substr(std::min(size_t(min_indent), lines[i].size())), TRIM_RIGHT).c_str());
    }
}

void AstNode::append_child(AstNode* node) {
    children.push_back(node);
    node->parent = this;
}

const char* AstNode::get_type_name() const {
    switch (type) {
    case AST_GRAMMAR:           return "GRAMMAR";
    case AST_COMMENT:           return "COMMENT";
    case AST_RULE:              return "RULE";
    case AST_RULE_NAME:         return "RULE_NAME";
    case AST_DIRECTIVE:         return "DIRECTIVE";
    case AST_DIRECTIVE_NAME:    return "DIRECTIVE_NAME";
    case AST_CODE:              return "CODE";
    case AST_SOURCE:            return "SOURCE";
    case AST_ERROR:             return "ERROR";
    case AST_PRIMARY:           return "PRIMARY";
    case AST_ALTERNATION:       return "ALTERNATION";
    case AST_SEQUENCE:          return "SEQUENCE";
    case AST_PREFIX_OP:         return "PREFIX_OP";
    case AST_POSTFIX_OP:        return "POSTFIX_OP";
    case AST_RULEREF:           return "RULEREF";
    case AST_REFNAME:           return "REFNAME";
    case AST_VAR:               return "VAR";
    case AST_STRING:            return "STRING";
    case AST_CHARCLASS:         return "CHARCLASS";
    case AST_DOT:               return "DOT";
    case AST_BACKREF:           return "BACKREF";
    case AST_GROUP:             return "GROUP";
    case AST_CAPTURE:           return "CAPTURE";
    default:
        Io::debug("ERROR: unexpected AST node type!\n");
        exit(2);
    }
}

void AstNode::print_ast(int level) const {
    const char* typeName = get_type_name();

    if (children.size() > 0) {
        Io::print("%*s%s:\n", 2*level, "", typeName);
        for (int i=0; i < children.size(); i++) {
            children[i]->print_ast(level + 1);
        }
    } else if (line >= 0) {
        Io::print("%*s%s: \"%s\" line=%ld, col=%ld\n", 2*level, "", typeName, text.c_str(), line, column);
    } else {
        Io::print("%*s%s: \"%s\" (optimized)\n", 2*level, "", typeName, text.c_str(), line, column);
    }
}

void AstNode::format_terminal() const {
    Io::print("%s", text.c_str());
}

void AstNode::format_grammar() const {
    if (children.size() > 0) {
        for (size_t i = 0; i< children.size(); i++) {
            children[i]->format();
        }
    }
}

void AstNode::format_string() const {
    char quote = Config::get<Config::QuoteType>("quotes") == Config::QT_SINGLE ? '\'' : '\"';
    char other = quote == '\"' ? '\'' : '\"';
    Io::print("%c", quote);
    char next = 0;
    for(size_t i = 0; i < text.size(); i++) {
        switch (text[i]) {
        case '\\':
            next = text[++i];
            Io::print("%s%c", next == other ? "" : "\\", next);
            break;
        case '"':
        case '\'':
            Io::print("%s%c", text[i] == quote ? "\\" : "", text[i]);
            break;
        default:
            Io::print("%c", text[i]);
            break;
        }
    }
    Io::print("%c", quote);
}

void AstNode::format_source() const {
    std::string trimmed = trim(text);

    bool has_newlines = trimmed.find_first_of('\n') != std::string::npos;
    bool is_c_directive = trimmed[0] == '#'; // PackCC doesn't currently support single line C directives like %header { #include "x.h" }

    int is_directive = parent->type == AST_DIRECTIVE;

    if (has_newlines || is_c_directive) {
        Io::print(" {\n");
        reindent(text, is_directive ? 4 : 8);
        Io::print("%s}", is_directive ? "" : "    ");
    } else {
        Io::print(" { %s }", trimmed.c_str());
    }
}

void AstNode::format_error() const {
    Io::print(" ~");
    format_source();
}

void AstNode::format_directive() const {
    AstNode* content = children[1];

    Io::print("%%");
    children[0]->format();
    if (content->type == AST_STRING) {
        Io::print(" ");
    }
    content->format();
    Io::print(parent->children.back() == this ? "\n" : "\n\n");
}

void AstNode::format_alternation() const {
    int multiline = children.size() > Config::get<int>("wrap-limit") && parent->type == AST_RULE;
    const char *delim = multiline ? "\n    / " : " / ";
    for (size_t i = 0; i < children.size(); i++) {
        if (i > 0) {
            Io::print("%s", delim);
        }
        children[i]->format();
    }
}

void AstNode::format_sequence() const {
    for (size_t i = 0; i < children.size(); i++) {
        if (i > 0) {
            Io::print(" ");
        }
        children[i]->format();
    }
}

void AstNode::format_primary() const {
    for (size_t i = 0; i < children.size(); i++) {
        children[i]->format();
    }
}

void AstNode::format_group(const char open, const char close) const {
    Io::print("%c", open);
    children[0]->format();
    Io::print("%c", close);
}

void AstNode::format_ruleref() const {
    children[0]->format();
    if (children.size() == 2) {
        // has variable
        Io::print(":");
        children[1]->format();
    }
}

void AstNode::format_rule() const {
    AstNode* body = children[1];
    bool hasAlternation = body->type == AST_ALTERNATION && body->children.size() > 1;
    Io::print("%s%s <- ", children[0]->text.c_str(), hasAlternation ? "\n   " : "");
    body->format();
    Io::print(parent->children.back() == this ? "\n" : "\n\n");
}

void AstNode::format_code() const {
    Io::print("%%%%\n%s\n", trim(text).c_str());
}

void AstNode::format_comment() const {
    // We have to recognize three different comment positions:
    //   1) top-level comment (parent is GRAMMAR) -> add newline, no indent
    //   2) at the end of sequence in top-level alternation -> no newline, no indent
    //   3) anywhere else -> add newline + indent
    const char* suffix = "\n        ";
    if (parent->type == AST_GRAMMAR) {
        suffix = "\n";
    } else {
        AstNode* parent_alternation = find_parent(AST_ALTERNATION);
        if (parent_alternation
            && parent_alternation->parent->type == AST_RULE
            && parent->children.back() == this /* this comment is at the end of sequence */)
        {
            suffix = "";
        }
    }

    Io::print("# %s%s", trim(text).c_str(), suffix);
}

AstNode* AstNode::find_parent(AstNodeType type) const {
    if (!parent) {
        return parent;
    } else if (parent->type == type) {
        return parent;
    } else {
        return parent->find_parent(type);
    }
}

std::vector<AstNode*> AstNode::find_all(const std::function <bool(const AstNode&)>& predicate, const bool global) const {
    if (global) {
        return find_parent(AST_GRAMMAR)->find_all(predicate);
    }
    std::vector<AstNode*> result;
    if (predicate(*this)) {
        result.push_back(const_cast<AstNode *>(this));
    }
    for (int i = 0; i < children.size(); i++) {
        std::vector<AstNode*> subresults = children[i]->find_all(predicate);
        if (!subresults.empty()) {
            result.insert(result.end(), subresults.begin(), subresults.end());
        }
    }
    return result;
}

void AstNode::remove_child(AstNode* child) {
    children.erase(std::find(children.begin(), children.end(), child));
    delete child;
}

void AstNode::format() const {
    switch (type) {
    case AST_GRAMMAR:           format_grammar(); break;
    case AST_COMMENT:           format_comment(); break;
    case AST_RULE:              format_rule(); break;
    case AST_DIRECTIVE:         format_directive(); break;
    case AST_CODE:              format_code(); break;
    case AST_PRIMARY:           format_primary(); break;
    case AST_ALTERNATION:       format_alternation(); break;
    case AST_SEQUENCE:          format_sequence(); break;
    case AST_RULEREF:           format_ruleref(); break;
    case AST_GROUP:             format_group('(', ')'); break;
    case AST_CAPTURE:           format_group('<', '>'); break;
    case AST_SOURCE:            format_source(); break;
    case AST_ERROR:             format_error(); break;
    case AST_STRING:            format_string(); break;

    // everything else just print verbatim:
    case AST_RULE_NAME:
    case AST_DIRECTIVE_NAME:
    case AST_VAR:
    case AST_REFNAME:
    case AST_PREFIX_OP:
    case AST_POSTFIX_OP:
    case AST_CHARCLASS:
    case AST_DOT:
    case AST_BACKREF:
        format_terminal();
        break;
    default:
        Io::debug("ERROR: unexpected AST node type!\n");
        exit(2);
    }
}

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
    Io::debug("  Removing comment\n");
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
    std::vector<AstNode*> uninlinable = rule->find_all([](const AstNode& node) {
        return node.type == AST_SOURCE
               || node.type == AST_VAR
               || node.type == AST_CAPTURE;
    });
    if (!uninlinable.empty()) {
        return 0;
    }

    std::string name = children[0]->text;
    std::vector<AstNode*> refs = find_all([name](const AstNode& node) {
        return node.type == AST_REFNAME && node.text == name;
    }, true);
    if (refs.empty() || refs.size() > Config::get<int>("inline-limit")) {
        return 0;
    }
    Io::debug("  Inlining rule '%s' at %ld site%s\n", name.c_str(), refs.size(), refs.size() > 1 ? "s" : "");

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
    }
    // delete the rule
    parent->remove_child(this);
    return refs.size();
}

int AstNode::optimize_grammar() {
    int total = 0;
    for (int optimized = -1, i = 1; optimized != 0; i++) {
        Io::debug("Optimization pass %d:\n", i);
        optimized = optimize_children();
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
    case AST_PRIMARY:           return optimize_children() + optimize_single_child();
    case AST_RULEREF:           return optimize_children() + optimize_single_child();
    case AST_SEQUENCE:          return optimize_children() + optimize_single_child() + optimize_strings();
    case AST_COMMENT:           return optimize_strip_comment();
    case AST_VAR:               return optimize_unused_variable();

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
    case AST_PREFIX_OP:
    case AST_POSTFIX_OP:
    case AST_CHARCLASS:
    case AST_DOT:
    case AST_BACKREF:
        return optimize_children();
    }
    Io::debug("ERROR: unexpected AST node type!\n");
    exit(2);
}

void AstNode::debug() {
    Io::print("### Original AST:\n");
    print_ast();
    Io::print("### Original formatted:\n");
    format();
    optimize();
    Io::print("### Optimized AST:\n");
    print_ast();
    Io::print("### Optimized formatted:\n");
    format();
}

AstNode::AstNode(AstNodeType type, std::string text, size_t line, size_t column) :
    text(text), type(type), line(line), column(column), parent(NULL)
{
    //~ printf("CREATE %p (size=%ld, parent=%p, type=%s)\n", this, children.size(), parent, getTypeName());
}

AstNode::AstNode(const AstNode& other, AstNode* parent) :
    text(other.text), type(other.type), line(other.line), column(other.column), parent(parent)
{
    for (int i = 0; i < other.children.size(); i++) {
        children.push_back(new AstNode(*other.children[i], this));
    }
    //~ printf("COPY %p -> %p (size=%ld, parent=%p, type=%s)\n", this, &other, children.size(), parent, getTypeName());
}

AstNode::~AstNode() {
    //~ printf("DELETE %p (size=%ld, parent=%p, type=%s)\n", this, children.size(), parent, getTypeName());
    for (size_t i = 0; i < children.size(); i++) {
        delete children[i];
    }
}
