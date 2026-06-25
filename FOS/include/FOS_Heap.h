#ifndef FOS_HEAP_H
#define FOS_HEAP_H

#include "FOS_Vec.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    FOS_Vec vec;
    int (*cmp_func)(const void *fst, const void *snd);
    void *swap_buf;
} FOS_Heap;

bool FOS_heap_init(FOS_Heap *heap, int (*cmp)(const void *fst, const void *snd), size_t elem_size);
bool FOS_heap_push(FOS_Heap *heap, const void *elem);
bool FOS_heap_pop(FOS_Heap *heap, void *elem);
bool FOS_heap_peek(const FOS_Heap *heap, void *elem);
bool FOS_heap_is_empty(const FOS_Heap *heap);
size_t FOS_heap_get_size(const FOS_Heap *heap);
void FOS_heap_free(FOS_Heap *heap);

#endif
