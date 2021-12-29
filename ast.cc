#include "ast.h"

#include <sstream>
#include <algorithm>
#include <climits>

extern int debug_mode;

enum TrimType {
    TRIM_LEFT = 1,
    TRIM_RIGHT = 2,
    TRIM_BOTH = TRIM_LEFT | TRIM_RIGHT
};

static string trim(const string& str, TrimType type = TRIM_BOTH) {
    size_t start = (type & TRIM_LEFT) ? str.find_first_not_of(" \t\r\n") : 0;
    size_t end = (type & TRIM_RIGHT) ? str.find_last_not_of(" \t\r\n") + 1 : str.size();
    if (start == string::npos) {
        start = 0;
    }
    return string(str.c_str() + start, end - start);
}

static void reindent(const string& str, int baseindent) {
    std::stringstream stream(str);
    string line;
    vector<string> lines;
    int min_indent = INT_MAX;
    while(std::getline(stream, line)){
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
        printf("%*s%s\n", baseindent + indent, "", trim(lines[i].substr(std::min(size_t(min_indent), lines[i].size())), TRIM_RIGHT).c_str());
    }
}

void AstNode::appendChild(AstNode* node) {
    children.push_back(node);
    node->parent = this;
}

const char* AstNode::getTypeName() {
    switch (type) {
    case AST_GRAMMAR:           return "GRAMMAR";
    case AST_COMMENT:           return "COMMENT";
    case AST_RULE:              return "RULE";
    case AST_RULE_NAME:         return "RULE_NAME";
    case AST_DIRECTIVE:         return "DIRECTIVE";
    case AST_DIRECTIVE_NAME:    return "DIRECTIVE_NAME";
    case AST_CODE:              return "CODE";
    case AST_SOURCE:            return "SOURCE";
    case AST_PRIMARY:           return "PRIMARY";
    case AST_ALTERNATION:       return "ALTERNATION";
    case AST_SEQUENCE:          return "SEQUENCE";
    case AST_PREFIX_OP:         return "PREFIX_OP";
    case AST_POSTFIX_OP:        return "POSTFIX_OP";
    case AST_RULEREF:           return "RULEREF";
    case AST_VAR:               return "VAR";
    case AST_STRING:            return "STRING";
    case AST_CHARCLASS:         return "CHARCLASS";
    case AST_DOT:               return "DOT";
    case AST_BACKREF:           return "BACKREF";
    case AST_GROUP:             return "GROUP";
    case AST_CAPTURE:           return "CAPTURE";
    default:                    return "UNKNOWN";
    }
}

void AstNode::print_ast(int level) {
    const char* typeName = getTypeName();

    if (children.size() > 0) {
        printf("%*s%s:\n", 2*level, "", typeName);
        for (int i=0; i < children.size(); i++) {
            children[i]->print_ast(level + 1);
        }
    } else if (line >= 0) {
        printf("%*s%s: \"%s\" line=%ld, col=%ld\n", 2*level, "", typeName, text.c_str(), line, column);
    } else {
        printf("%*s%s: \"%s\" (optimized)\n", 2*level, "", typeName, text.c_str(), line, column);
    }
}

void AstNode::format_terminal() {
    printf("%s", text.c_str());
}

void AstNode::format_grammar() {
    if (children.size() > 0) {
        for (size_t i = 0; i< children.size(); i++) {
            children[i]->format();
        }
    }
}

void AstNode::format_string() {
    // TODO: configurable quotes
    printf("\"");
    char next = 0;
    for(size_t i = 0; i < text.size(); i++) {
        switch (text[i]) {
        case '\\':
            next = text[++i];
            printf("%s%c", next == '\'' ? "" : "\\", next);
            break;
        case '"':
            printf("\\\"");
            break;
        default:
            printf("%c", text[i]);
            break;
        }
    }
    printf("\"");
}

void AstNode::format_source() {
    string trimmed = trim(text);

    bool hasNewlines = text.find_first_of('\n') == string::npos;

    int is_directive = parent->type == AST_DIRECTIVE;

    if (hasNewlines) {
        printf(" { %s }", trimmed.c_str());
    } else {
        printf(" {\n");
        reindent(text, is_directive ? 4 : 8);
        printf("%s}", is_directive ? "" : "    ");
    }
}

void AstNode::format_directive() {
    AstNode* content = children[1];

    printf("%%");
    children[0]->format();
    if (content->type == AST_STRING) {
        printf(" ");
    }
    content->format();
    printf("\n\n");
}

void AstNode::format_alternation() {
    // TODO: configurable wrapping (higher children.size() treshold)
    int multiline = children.size() > 1 && parent->type == AST_RULE;
    const char *delim = multiline ? "\n    / " : " / ";
    for (size_t i = 0; i < children.size(); i++) {
        if (i > 0) {
            printf("%s", delim);
        }
        children[i]->format();
    }
}

void AstNode::format_sequence() {
    for (size_t i = 0; i < children.size(); i++) {
        if (i > 0) {
            printf(" ");
        }
        children[i]->format();
    }
}

void AstNode::format_primary() {
    for (size_t i = 0; i < children.size(); i++) {
        children[i]->format();
    }
}

void AstNode::format_group(const char open, const char close) {
    printf("%c", open);
    children[0]->format();
    printf("%c", close);
}

void AstNode::format_ruleref() {
    if (children.size() > 0){
        // has variable
        children[0]->format();
        printf(":");
    }
    printf("%s", text.c_str());
}

void AstNode::format_rule() {
    AstNode* body = children[1];
    bool hasAlternation = body->type == AST_ALTERNATION && body->children.size() > 1;
    printf("%s%s <- ", children[0]->text.c_str(), hasAlternation ? "\n   " : "");
    body->format();
    printf("\n\n");
}

void AstNode::format_code() {
    printf("%%%%\n%s\n", trim(text).c_str());
}

void AstNode::format_comment() {
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

    printf("# %s%s", trim(text).c_str(), suffix);
}

AstNode* AstNode::find_parent(ast_node_type_t type) {
    if (!parent) {
        return parent;
    } else if (parent->type == type) {
        return parent;
    } else {
        return parent->find_parent(type);
    }
}

void AstNode::format() {
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
    case AST_STRING:            format_string(); break;

    // everything else just print verbatim:
    case AST_RULE_NAME:
    case AST_DIRECTIVE_NAME:
    case AST_VAR:
    case AST_PREFIX_OP:
    case AST_POSTFIX_OP:
    case AST_CHARCLASS:
    case AST_DOT:
    case AST_BACKREF:
        format_terminal();
        break;
    }
}

int AstNode::optimize_strings() {
    int optimized = 0;
    for (size_t i = 1; i < children.size(); i++) {
        AstNode* first = children[i-1];
        AstNode* second = children[i];
        if (first->type == AST_STRING && second->type == AST_STRING) {
            debug_mode && fprintf(stderr, "  Concatenating adjacent strings '%s' and '%s'\n", first->text.c_str(), second->text.c_str());
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
    debug_mode && fprintf(stderr, "  Removing unnecessary node of type %s\n", getTypeName());
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
    debug_mode && fprintf(stderr, "  Removing comment\n");
    parent->children.erase(std::find(parent->children.begin(), parent->children.end(), this));
    return 1;
}

int AstNode::optimize_grammar() {
    int total = 0;
    for (int optimized = -1, i = 0; optimized != 0; i++) {
        debug_mode && fprintf(stderr, "Optimization pass %d:\n", i);
        optimized = optimize_children();
        total += optimized;
        debug_mode && fprintf(stderr, " => %d optimization%s done in pass %d (%d total)\n", optimized, optimized == 1 ? "" : "s", i, total);
    };
    return total;
}

int AstNode::optimize_sequence() {
    return optimize_single_child() + optimize_strings() + optimize_children();
}

int AstNode::optimize_alternation() {
    return optimize_single_child() + optimize_children();
}

int AstNode::optimize_primary() {
    return optimize_single_child() + optimize_children();
}

int AstNode::optimize() {
    switch (type) {
    case AST_GRAMMAR:           return optimize_grammar();
    case AST_PRIMARY:           return optimize_primary();
    case AST_ALTERNATION:       return optimize_alternation();
    case AST_SEQUENCE:          return optimize_sequence();
    case AST_COMMENT:           return optimize_strip_comment();

    // everything else just call optimize on children
    case AST_RULE:
    case AST_DIRECTIVE:
    case AST_CODE:
    case AST_SOURCE:
    case AST_STRING:
    case AST_RULEREF:
    case AST_GROUP:
    case AST_CAPTURE:
    case AST_RULE_NAME:
    case AST_DIRECTIVE_NAME:
    case AST_VAR:
    case AST_PREFIX_OP:
    case AST_POSTFIX_OP:
    case AST_CHARCLASS:
    case AST_DOT:
    case AST_BACKREF:
        return optimize_children();
    }
    throw "ERROR: unexpected AST node type!";
}
