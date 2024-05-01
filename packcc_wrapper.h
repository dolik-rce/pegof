#define VOID_VALUE (~(size_t)0)

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

void string_array__init(string_array_t *array);
void string_array__add(string_array_t *array, const char *str, size_t len);
void string_array__term(string_array_t *array);

void *create_context(const char *ipath, const char *opath, const string_array_t *dirs, const options_t *opts);
void destroy_context(void *ctx);

bool parse(void *ctx);
bool generate(void *ctx);

bool_t unescape_string(char *str, bool_t cls);
size_t utf8_to_utf32(const char *seq, int *out);
}
