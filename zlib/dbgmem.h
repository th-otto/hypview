#ifndef __DBGMEM_H__
#define __DBGMEM_H__

#define z_malloc(size) malloc(size)
#define z_calloc(nitems, size) calloc(nitems, size)
#define z_strdup(str) strdup(str)
#define z_free(block) free(block)
#define z_realloc(block, newsize) realloc(block, newsize)

#endif /* __DBGMEM_H__ */

