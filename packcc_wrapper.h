
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

void *create_context(const char *iname, const char *oname, const options_t *opts);
void destroy_context(void *ctx);

bool parse(void *ctx);
bool generate(void *ctx);

bool_t unescape_string(char *str, bool_t cls);
size_t utf8_to_utf32(const char *seq, int *out);
}
