#define main disabled_main
#define fprintf fprintf_wrapped
#define vfprintf vfprintf_wrapped
#define static
#include "packcc.c"
#undef static
#undef vfprintf
#undef fprintf
#undef main
