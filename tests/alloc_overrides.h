#ifndef ALLOC_OVERRIDES_H
#define ALLOC_OVERRIDES_H

/*
 * This header is force-included only while compiling the project modules for
 * allocation-failure tests. Including stdlib.h first prevents these macros
 * from rewriting the standard library's own function declarations.
 */
#include <stddef.h>
#include <stdlib.h>

void *alloc_tracker_malloc(size_t size);
void *alloc_tracker_calloc(size_t count, size_t size);
void *alloc_tracker_realloc(void *pointer, size_t size);
void alloc_tracker_free(void *pointer);

#define malloc alloc_tracker_malloc
#define calloc alloc_tracker_calloc
#define realloc alloc_tracker_realloc
#define free alloc_tracker_free

#endif
