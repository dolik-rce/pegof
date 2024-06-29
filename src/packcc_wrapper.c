#define main disabled_main
#define fprintf fprintf_wrapped
#define vfprintf vfprintf_wrapped
#define static
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#include "packcc.c"
#pragma GCC diagnostic push
#undef static
#undef vfprintf
#undef fprintf
#undef main
