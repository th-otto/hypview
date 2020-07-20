#ifndef _SPARSESET_H
#define _SPARSESET_H
#include <stdlib.h>

typedef struct {
    unsigned members;
    unsigned *sparse;
    unsigned *dense;
    unsigned *counts;
} Set;

extern Set *langid_alloc_set(size_t size);
extern void langid_free_set(Set *s);
extern void langid_clear_set(Set *s);
extern void langid_add_set(Set *s, unsigned key, unsigned val);

#endif
