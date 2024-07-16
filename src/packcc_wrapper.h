#pragma once

extern "C" {
typedef enum bool_tag {
    FALSE = 0,
    TRUE
} bool_t;

typedef struct options_tag {
    bool_t ascii;
    bool_t lines;
    bool_t debug;
} options_t;

typedef struct string_array_tag {
    char **buf;
    size_t max;
    size_t len;
} string_array_t;

void pcc_array_init(string_array_t *array);
void pcc_array_add(string_array_t *array, const char *str, size_t len);
void pcc_array_term(string_array_t *array);

bool_t pcc_unescape_string(char *str, bool_t cls);
size_t pcc_utf8_to_utf32(const char *seq, int *out);

bool_t pcc_process(const char *ipath, const char *opath, const string_array_t *dirs, const options_t *opts);
}
