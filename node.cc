#include "node.h"

Node::operator bool() {
    return valid;
}

Node* Node::operator[](int index) {
    return this;
}

long Node::size() const { return 1; }

std::string Node::to_c_string(std::string str) const {
    std::string result;
    for (int i = 0; i < str.size(); i++) {
        switch (str[i]) {
        case '\x00': result += "\\0"; break;
        case '\x07': result += "\\a"; break;
        case '\x08': result += "\\b"; break;
        case '\x0c': result += "\\f"; break;
        case '\x0a': result += "\\n"; break;
        case '\x0d': result += "\\r"; break;
        case '\x09': result += "\\t"; break;
        case '\x0b': result += "\\v"; break;
        case '\\':   result += "\\\\"; break;
        case '\'':   result += "\\\'"; break;
        case '\"':   result += "\\\""; break;
        default:
            if (str[i] >= '\x20' && str[i] < '\x7f') {
                result += str[i];
            } else {
                std::stringstream stream;
                stream << std::setfill ('0') << std::setw(2) << std::hex << (int)(unsigned char)str[i];
                result += "\\x" + stream.str();
            }
        }
    }
    return result;
}
