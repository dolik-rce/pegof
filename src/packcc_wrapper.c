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

#include "capi.h"

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

bool_t pcc_match_quoted(void* parser, void* result) {
    char* input;
    size_t len;
    unsigned long pos;
    convert_parser(parser, &input, &len, &pos);
    char_array_t buf = { input, len, len };
    input_state_t input_state = {
        "tmp",           // char *path;        /* the path name of the PEG file being parsed; "<stdin>" if stdin */
        dev_null(),      // FILE *file;        /* the input file stream of the PEG file */
        FALSE,           // bool_t ascii;      /* UTF-8 support is disabled if true  */
        CODE_FLAG__NONE, // code_flag_t flags; /* the bitwise flags to control code generation; updated during PEG parsing */
        0,               // size_t errnum;     /* the current number of PEG parsing errors */
        0,               // size_t linenum;    /* the current line number (0-based) */
        0,               // size_t charnum;    /* the number of characters in the current line that are already flushed (0-based, UTF-8 support if not disabled) */
        0,               // size_t linepos;    /* the beginning position in the PEG file of the current line */
        0,               // size_t bufpos;     /* the position in the PEG file of the first character currently buffered */
        pos,             // size_t bufcur;     /* the current parsing position in the character buffer */
        buf,             // char_array_t buffer;   /* the character buffer */
        NULL             // input_state_t *parent; /* the input state of the parent PEG file that imports the input; just a reference */
    };
    bool_t res = FALSE;
    if (match_quotation_single(&input_state) || match_quotation_double(&input_state) || match_character_class(&input_state)) {
        char* tmp = strndup_e(input_state.buffer.buf + pos + 1, input_state.bufcur - pos - 2);
        unescape_string(tmp, FALSE);
        store_string(result, tmp);
        free(tmp);
        update_parser(parser, input_state.bufcur);
        res = TRUE;
    }
    fclose(input_state.file);
    return res;
}

bool_t pcc_process(const char *ipath, const char *opath, const string_array_t *dirs, const options_t *opts) {
    void *const ctx = create_context(ipath, opath, dirs, opts);
    bool_t result = parse(ctx) && generate(ctx);
    destroy_context(ctx);
    return result;
}
