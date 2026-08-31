#include "FOS_Heap.h"
#include "FOS_Memory.h"
#include <string.h>
#include <stdint.h>

bool FOS_heap_init(FOS_Heap *heap, int (*cmp)(const void *fst, const void *snd), size_t elem_size)
{
    if (heap == NULL || cmp == NULL || elem_size == 0)
        return false;

    heap->vec = FOS_vec_new(elem_size);

    if (heap->vec.data == NULL)
        return false;

    heap->cmp_func = cmp;

    heap->swap_buf = FOS_alloc(elem_size);

    if (heap->swap_buf == NULL)
    {
        FOS_vec_free(&heap->vec);
        return false;
    }

    return true;
}

static size_t FOS_heap_parent(size_t index)
{
    return (index - 1) / 2;
}

static size_t FOS_heap_left(size_t index)
{
    return 2 * index + 1;
}

static size_t FOS_heap_right(size_t index)
{
    return 2 * index + 2;
}

static bool FOS_heap_sift_up(FOS_Heap *heap, size_t index)
{
    if (heap == NULL)
        return false;

    char *temp = heap->swap_buf;

    if (temp == NULL)
        return false;

    while (index > 0)
    {
        char *curr = (char *)heap->vec.data + index * heap->vec.elem_size;
        char *parent = (char *)heap->vec.data + FOS_heap_parent(index) * heap->vec.elem_size;

        if (heap->cmp_func(curr, parent) >= 0)
            break;

        memcpy(temp, parent, heap->vec.elem_size);
        memcpy(parent, curr, heap->vec.elem_size);
        memcpy(curr, temp, heap->vec.elem_size);

        index = FOS_heap_parent(index);
    }

    return true;
}

bool FOS_heap_push(FOS_Heap *heap, const void *elem)
{
    if (heap == NULL || elem == NULL)
        return false;

    if (!FOS_vec_push(&heap->vec, elem))
        return false;

    if (!FOS_heap_sift_up(heap, heap->vec.size - 1))
    {
        FOS_vec_erase_at(&heap->vec, heap->vec.size - 1);
        return false;
    }

    return true;
}

static bool FOS_heap_left_exists(FOS_Heap *heap, size_t index)
{
    if (heap == NULL)
        return false;

    if (FOS_heap_left(index) < heap->vec.size)
        return true;

    return false;
}

static bool FOS_heap_right_exists(FOS_Heap *heap, size_t index)
{
    if (heap == NULL)
        return false;

    if (FOS_heap_right(index) < heap->vec.size)
        return true;

    return false;
}


static bool FOS_heap_sift_down(FOS_Heap *heap, size_t index)
{
    if (heap == NULL)
        return false;

    char *temp = heap->swap_buf;

    if (temp == NULL)
        return false;

    bool is_left = false;
    bool is_right = false;

    while ((is_left = FOS_heap_left_exists(heap, index)) || (is_right = FOS_heap_right_exists(heap, index)))
    {
        char *curr = (char *)heap->vec.data + index * heap->vec.elem_size;
        char *left = NULL;
        char *right = NULL;

        if (is_left)
            left = (char *)heap->vec.data + FOS_heap_left(index) * heap->vec.elem_size;
        
        if (is_right)
            right = (char *)heap->vec.data + FOS_heap_right(index) * heap->vec.elem_size;

        char *smallest = curr;

        if (is_left && heap->cmp_func(left, smallest) < 0)
            smallest = left;

        if (is_right && heap->cmp_func(right, smallest) < 0)
            smallest = right;

        if (smallest == curr)
            break;

        memcpy(temp, smallest, heap->vec.elem_size);
        memcpy(smallest, curr, heap->vec.elem_size);
        memcpy(curr, temp, heap->vec.elem_size);

        if (smallest == right)
            index = FOS_heap_right(index);
        else if (smallest == left)
            index = FOS_heap_left(index);
    }

    return true;
}

bool FOS_heap_pop(FOS_Heap *heap, void *elem)
{
    if (heap == NULL || heap->vec.size == 0 || elem == NULL)
        return false;

    memcpy(elem, heap->vec.data, heap->vec.elem_size);

    memmove((char *)heap->vec.data, 
            (char *)heap->vec.data + (heap->vec.size - 1) * heap->vec.elem_size, 
             heap->vec.elem_size);

    --heap->vec.size;

    return FOS_heap_sift_down(heap, 0);
}

bool FOS_heap_peek(const FOS_Heap *heap, void *elem)
{
    if (heap == NULL || elem == NULL)
        return false;

    if (heap->vec.size > 0)
        memcpy(elem, heap->vec.data, heap->vec.elem_size);
    else
        return false;

    return true;
}

bool FOS_heap_is_empty(const FOS_Heap *heap)
{
    if (heap == NULL)
        return false;

    if (heap->vec.size == 0)
        return true;

    return false;
}

size_t FOS_heap_get_size(const FOS_Heap *heap)
{
    if (heap == NULL)
        return SIZE_MAX;

    return heap->vec.size;
}

void FOS_heap_free(FOS_Heap *heap)
{
    if (heap == NULL)
        return;

    FOS_free(heap->swap_buf);
    FOS_vec_free(&heap->vec);

    memset(heap, 0, sizeof(*heap));
}
