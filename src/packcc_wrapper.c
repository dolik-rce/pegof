#define main disabled_main
#define fprintf fprintf_wrapped
#define vfprintf vfprintf_wrapped
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#include "packcc.c"
#pragma GCC diagnostic push
#undef vfprintf
#undef fprintf
#undef main

void pcc_array_init(string_array_t *array) {
    string_array__init(array);
}

void pcc_array_add(string_array_t *array, const char *str, size_t len) {
    string_array__add(array, str, len);
}

void pcc_array_term(string_array_t *array) {
    string_array__term(array);
}

bool_t pcc_unescape_string(char *str, bool_t cls) {
    return unescape_string(str, cls);
}

size_t pcc_utf8_to_utf32(const char *seq, int *out) {
    return utf8_to_utf32(seq, out);
}

bool_t pcc_process(const char *ipath, const char *opath, const string_array_t *dirs, const options_t *opts) {
    void *const ctx = create_context(ipath, opath, dirs, opts);
    bool_t result = parse(ctx) && generate(ctx);
    destroy_context(ctx);
    return result;
}
