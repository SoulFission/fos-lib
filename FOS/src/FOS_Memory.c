#include "FOS_Memory.h"
#include <stdlib.h>

FOS_AllocatorBackend fos_allocator;

static void *std_alloc(size_t size)
{
    return malloc(size);
}

static void std_free(void *ptr)
{
    free(ptr);
}

static void *std_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

static void *std_calloc(size_t count, size_t size)
{
    return calloc(count, size);
}

void FOS_mem_init_std(void)
{
    fos_allocator.alloc = std_alloc;
    fos_allocator.free = std_free;
    fos_allocator.realloc = std_realloc;
    fos_allocator.calloc = std_calloc;
}

void *FOS_alloc(size_t size)
{
    if (!fos_allocator.alloc)
        FOS_mem_init_std();

    return fos_allocator.alloc(size);
}

void  FOS_free(void *ptr)
{
    if (!fos_allocator.alloc)
        FOS_mem_init_std();

    fos_allocator.free(ptr);
}

void *FOS_realloc(void *ptr, size_t size)
{
    if (!fos_allocator.alloc)
        FOS_mem_init_std();

    return fos_allocator.realloc(ptr, size);
}

void *FOS_calloc(size_t count, size_t size)
{
    if (!fos_allocator.alloc)
        FOS_mem_init_std();

    return fos_allocator.calloc(count, size);
}
