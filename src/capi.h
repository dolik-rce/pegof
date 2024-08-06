#pragma once
#include <stdio.h>

FILE* dev_null(void);

void convert_parser(void* parser, char** input, size_t* len, unsigned long* pos);
void update_parser(void* parser, unsigned long pos);

void store_string(void* str, char* value);
