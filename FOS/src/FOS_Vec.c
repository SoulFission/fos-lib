#include "FOS_Vec.h"

FOS_Vec FOS_vec_new(size_t elem_size)
{
    if (elem_size == 0)
        return (FOS_Vec) { 0 };

    FOS_Vec vec = { 0 };

    vec.elem_size = elem_size;
    vec.capacity = FOS_VEC_INIT_CAP;
    vec.data = calloc(vec.capacity, elem_size);

    if (vec.data == NULL)
        return (FOS_Vec) { 0 };

    return vec;
}

void FOS_vec_free(FOS_Vec *vec)
{
    if (vec == NULL)
        return;

    free(vec->data);

    *vec = (FOS_Vec) { 0 };
}

void FOS_vec_clear(FOS_Vec *vec)
{
    if (vec == NULL || vec->data == NULL)
        return;

    vec->size = 0;
}

bool FOS_vec_reserve(FOS_Vec *vec, size_t new_capacity)
{
    if (vec == NULL || vec->data == NULL)
        return false;

    if (new_capacity <= vec->capacity)
        return true;

    void *new_data = realloc(vec->data, new_capacity * vec->elem_size);

    if (new_data == NULL)
        return false;

    vec->data     = new_data;
    vec->capacity = new_capacity;

    return true;
}

bool FOS_vec_push(FOS_Vec *vec, const void *elem)
{
    if (vec == NULL || vec->data == NULL || elem == NULL)
        return false;

    if (vec->size == vec->capacity)
    {
        size_t new_capacity = vec->capacity ? vec->capacity * 2
                                            : FOS_VEC_INIT_CAP;

        if (!FOS_vec_reserve(vec, new_capacity))
            return false;
    }

    void *dest = (char *)vec->data + vec->size * vec->elem_size;

    memmove(dest, elem, vec->elem_size);

    vec->size++;

    return true;
}

void *FOS_vec_at(FOS_Vec *vec, size_t index)
{
    if (vec == NULL || vec->data == NULL || index >= vec->size)
        return NULL;

    return (char *)vec->data + index * vec->elem_size;
}

void *FOS_vec_data(FOS_Vec *vec)
{
    if (vec == NULL || vec->data == NULL)
        return NULL;

    return vec->data;
}

bool FOS_vec_pop(FOS_Vec *vec, void *out_elem)
{
    if (vec == NULL || vec->data == NULL || vec->size == 0 || out_elem == NULL)
        return false;

    vec->size--;

    void *src = (char *)vec->data + vec->size * vec->elem_size;
    memcpy(out_elem, src, vec->elem_size);

    return true;
}

bool FOS_vec_copy(FOS_Vec *dst, const FOS_Vec *src)
{
    if (dst == NULL || dst->data == NULL ||
        src == NULL || src->data == NULL)
        return false;

    if (dst == src)
        return true;

    if (dst->elem_size != src->elem_size)
        return false;

    if (src->size > dst->capacity)
    {
        size_t new_capacity = dst->capacity ? dst->capacity : FOS_VEC_INIT_CAP;

        while (new_capacity < src->size)
            new_capacity *= 2;

        if (!FOS_vec_reserve(dst, new_capacity))
            return false;
    }
    
    memmove(dst->data, src->data, src->elem_size * src->size);
    
    dst->size = src->size;

    return true;
}

bool FOS_vec_shrink_to_fit(FOS_Vec *vec)
{
    if (vec == NULL || vec->data == NULL)
        return false;

    if (vec->size == vec->capacity)
        return true;

    void *ptr = realloc(vec->data, vec->size * vec->elem_size);

    if (ptr == NULL)
        return false;
    
    vec->data = ptr;
    vec->capacity = vec->size;

    return true;
}

bool FOS_vec_resize(FOS_Vec *vec, size_t new_size)
{
    if (vec == NULL || vec->data == NULL)
        return false;

    if (new_size > vec->capacity) 
    {
        size_t new_capacity = vec->capacity ? vec->capacity * 2 
                                            : FOS_VEC_INIT_CAP;

        while (new_capacity < new_size)
            new_capacity *= 2; 

        if (!FOS_vec_reserve(vec, new_capacity))
            return false;
    }

    if (new_size > vec->size)
    {
        size_t count = new_size - vec->size;
        void *dst = (char *)vec->data + vec->size * vec->elem_size;

        memset(dst, 0, count * vec->elem_size);
    }

    vec->size = new_size;

    return true;
}

bool FOS_vec_insert(FOS_Vec *vec, size_t index, const void *elem)
{
    if (vec == NULL || vec->data == NULL || index > vec->size || elem == NULL)
        return false;

    if (index == vec->size)
        return FOS_vec_push(vec, elem);

    if (vec->size == vec->capacity)
    {
        size_t new_capacity = vec->capacity ? vec->capacity * 2 
                                            : FOS_VEC_INIT_CAP;

        if (!FOS_vec_reserve(vec, new_capacity))
            return false;
    }
    
    char *ptr = vec->data;

    memmove(ptr + (index + 1) * vec->elem_size, 
            ptr + index * vec->elem_size, 
            (vec->size - index) * vec->elem_size);
    memcpy(ptr + index * vec->elem_size, elem, vec->elem_size);
    vec->size++;

    return true;
}

bool FOS_vec_erase_range(FOS_Vec *vec, size_t start, size_t end)
{
    if (vec == NULL || vec->data == NULL)
        return false;

    if (start > end || end >= vec->size)
        return false;

    char *vd = vec->data;
    size_t es = vec->elem_size;

    size_t count = end - start + 1;
    size_t tail = vec->size - (end + 1);

    memmove(vd + start * es, vd + (end + 1) * es, tail * es);

    vec->size -= count;

    return true;
}

bool FOS_vec_erase_at(FOS_Vec *vec, size_t i)
{
    if (vec == NULL || vec->data == NULL)
        return false;

    if (i >= vec->size)
        return false;
    
    return FOS_vec_erase_range(vec, i, i);
}

bool FOS_vec_erase_unordered(FOS_Vec *vec, size_t i)
{
    if (vec == NULL || vec->data == NULL || i >= vec->size)
        return false;

    if (i != vec->size - 1)
    {
        memcpy((char*)vec->data + i * vec->elem_size,
               (char*)vec->data + (vec->size - 1) * vec->elem_size,
               vec->elem_size);
    }

    vec->size--;
    return true;
}

bool FOS_vec_sort(FOS_Vec *vec, FOS_Compare cmp)
{
    if (vec == NULL || vec->data == NULL || cmp == NULL)
        return false;

    qsort(vec->data, vec->size, vec->elem_size, cmp);

    return true;
}
