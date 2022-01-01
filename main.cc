#include "parser.h"

#include <stdio.h>
#include <string.h>

bool debug_mode = false;

int main(int argc, char **argv) {
    Config conf(argc, argv);
    Parser parser = Parser(conf);
    return parser.parse_all();
}
