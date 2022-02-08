#include "ast.h"
#include "config.h"
#include "io.h"

#include <climits>
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
    case AST_CHARCLASS:         format_character_class(); break;
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
    case AST_DOT:
    case AST_BACKREF:
        format_terminal();
        break;
    default:
        Io::debug("ERROR: unexpected AST node type!\n");
        exit(2);
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

void AstNode::format_character_class() const {
    Io::print("[%s]", text.c_str());
}

void AstNode::format_source() const {
    std::string trimmed = trim(text);

    bool has_newlines = trimmed.find_first_of('\n') != std::string::npos;
    bool is_c_directive = trimmed[0] == '#'; // PackCC doesn't support single line C directives e.g.: %header { #include "x.h" }

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
