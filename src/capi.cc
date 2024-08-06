#include "utils.h"
#include "parser.h"
#include <string>
#include <stdio.h>

extern "C" {

FILE* dev_null(void) {
    const std::string path = TempDir::get("null");
    fclose(fopen(path.c_str(), "w"));
    return fopen(path.c_str(), "r");
}

void convert_parser(void* parser, char** input, size_t* len, unsigned long* pos) {
    Parser* p = (Parser*)parser;
    *input = const_cast<char *>(p->input.c_str());
    *len = p->input.size();
    *pos = p->pos;
}

void update_parser(void* parser, unsigned long pos) {
    Parser* p = (Parser*)parser;
    p->pos = pos;
}

void store_string(void* str, char* value) {
    std::string* s = (std::string*)str;
    *s = value;
}

}
