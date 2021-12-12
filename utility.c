#include "utility.h"

#include <stdlib.h>

#ifndef CHAR_ARRAY_MIN
#define CHAR_ARRAY_MIN 256 /* the minimum number of char_array_t elements to be allocated */
#endif

#ifndef SIZE_T_ARRAY_MIN
#define SIZE_T_ARRAY_MIN 16 /* the minimum number of size_t_array_t elements to be allocated */
#endif

void char_array__initialize(char_array_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

void char_array__finalize(char_array_t *obj) {
    free(obj->p);
}

bool_t char_array__resize(char_array_t *obj, size_t size) {
    if (obj->m < size) {
        size_t m = obj->m;
        if (m == 0) m = CHAR_ARRAY_MIN;
        while (m < size && m != 0) m <<= 1;
        if (m == 0) m = size; /* in case of shift overflow */
        char *const p = (char *)realloc(obj->p, m);
        if (p == NULL) return BOOL_FALSE;
        obj->p = p;
        obj->m = m;
    }
    obj->n = size;
    return BOOL_TRUE;
}

void size_t_array__initialize(size_t_array_t *obj) {
    obj->m = 0;
    obj->n = 0;
    obj->p = NULL;
}

void size_t_array__finalize(size_t_array_t *obj) {
    free(obj->p);
}

bool_t size_t_array__resize(size_t_array_t *obj, size_t size) {
    if (obj->m < size) {
        size_t m = obj->m;
        if (m == 0) m = SIZE_T_ARRAY_MIN;
        while (m < size && m != 0) m <<= 1;
        if (m == 0) m = size; /* in case of shift overflow */
        size_t *const p = (size_t *)realloc(obj->p, sizeof(size_t) * m);
        if (p == NULL) return BOOL_FALSE;
        obj->p = p;
        obj->m = m;
    }
    obj->n = size;
    return BOOL_TRUE;
}
