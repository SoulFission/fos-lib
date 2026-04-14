#ifndef FOS_MEMORY_H
#define FOS_MEMORY_H

#include <stddef.h>

typedef struct {
    void *(*alloc)(size_t);
    void  (*free)(void *);
    void *(*realloc)(void *, size_t);
    void *(*calloc)(size_t, size_t);
} FOS_AllocatorBackend;

extern FOS_AllocatorBackend fos_allocator;

void FOS_mem_init_std(void);

void *FOS_alloc(size_t size);
void  FOS_free(void *ptr);
void *FOS_realloc(void *ptr, size_t size);
void *FOS_calloc(size_t count, size_t size);

#endif
