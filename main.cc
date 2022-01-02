#include "parser.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    Config conf(argc, argv);
    Parser parser = Parser(conf);
    return parser.parse_all();
}
