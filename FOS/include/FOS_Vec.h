#ifndef FOS_VEC_H
#define FOS_VEC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FOS_VEC_DEFINE(TYPE, NAME) \
typedef FOS_Vec FOS_Vec_##NAME; \
\
static inline FOS_Vec_##NAME FOS_vec_##NAME##_new(void) \
{ \
    return FOS_vec_new(sizeof(TYPE)); \
} \
static inline void FOS_vec_##NAME##_free(FOS_Vec_##NAME *vec) \
{\
    FOS_vec_free(vec);  \
}\
\
static inline void FOS_vec_##NAME##_clear(FOS_Vec_##NAME *vec) \
{\
    FOS_vec_clear(vec);  \
}\
static inline bool FOS_vec_##NAME##_reserve(FOS_Vec_##NAME *vec, size_t new_capacity)  \
{ \
    return FOS_vec_reserve(vec, new_capacity);  \
} \
\
static inline bool FOS_vec_##NAME##_push(FOS_Vec_##NAME *vec, TYPE value) \
{ \
    return FOS_vec_push(vec, &value); \
} \
\
static inline TYPE *FOS_vec_##NAME##_at(FOS_Vec_##NAME *vec, size_t i) \
{ \
    return (TYPE *)FOS_vec_at(vec, i); \
} \
\
static inline TYPE *FOS_vec_##NAME##_data(FOS_Vec_##NAME *vec) \
{ \
    return (TYPE *)FOS_vec_data(vec); \
}\
static inline bool FOS_vec_##NAME##_pop(FOS_Vec_##NAME *vec, TYPE *out_elem) \
\
{ \
    return FOS_vec_pop(vec, out_elem); \
} \
\
static inline bool FOS_vec_##NAME##_copy(FOS_Vec_##NAME *dst, const FOS_Vec_##NAME *src) \
{ \
    return FOS_vec_copy(dst, src);  \
} \
\
static inline bool FOS_vec_##NAME##_resize(FOS_Vec_##NAME *v, size_t new_size)\
{\
    return FOS_vec_resize(v, new_size); \
}\
static inline bool FOS_vec_##NAME##_shrink_to_fit(FOS_Vec_##NAME *vec) \
{ \
    return FOS_vec_shrink_to_fit(vec);  \
} \
\
static inline bool FOS_vec_##NAME##_insert(FOS_Vec_##NAME *vec, size_t index, const TYPE *elem) \
{\
    return FOS_vec_insert(vec, index, elem);  \
}\
\
static inline bool FOS_vec_##NAME##_erase_range(FOS_Vec_##NAME *vec, size_t start, size_t end) \
{ \
    return FOS_vec_erase_range(vec, start, end); \
} \
\
static inline bool FOS_vec_##NAME##_erase_at(FOS_Vec_##NAME *vec, size_t i) \
{ \
    return FOS_vec_erase_at(vec, i); \
} \
\
static inline bool FOS_vec_##NAME##_erase_unordered(FOS_Vec_##NAME *vec, size_t i) \
{ \
    return FOS_vec_erase_unordered(vec, i); \
} \
int FOS_vec_##NAME##_compare(const void *fst, const void *snd) \
{\
    if (*(TYPE *)fst < *(TYPE *)snd) \
        return -1; \
    if (*(TYPE *)fst > *(TYPE *)snd) \
        return 1; \
    return 0;\
}\
static inline bool FOS_vec_##NAME##_sort(FOS_Vec_##NAME *v) \
{\
    return FOS_vec_sort(v, FOS_vec_##NAME##_compare);\
}


#define FOS_VEC_FOR_EACH(type, vec_ptr, it)                          \
    for (type *it = (type *)(vec_ptr)->data,                         \
              *it##_end = (type *)((char *)(vec_ptr)->data +         \
                                   (vec_ptr)->size * (vec_ptr)->elem_size); \
         it < it##_end;                                              \
         ++it)

#define FOS_VEC_FOR_EACH_REV(type, vec_ptr, it)                      \
    if ((vec_ptr)->size > 0)                                         \
        for (type *it = (type *)((char *)(vec_ptr)->data +               \
                                 ((vec_ptr)->size - 1) * (vec_ptr)->elem_size), \
                  *it##_begin = (type *)(vec_ptr)->data;                 \
            it >= it##_begin;                                           \
            --it)

#define FOS_VEC_FOR_EACH_RAW(vec_ptr, it)                            \
    for (char *it = (char *)(vec_ptr)->data,                         \
              *it##_end = (char *)(vec_ptr)->data +                  \
                           (vec_ptr)->size * (vec_ptr)->elem_size;   \
         it < it##_end;                                              \
         it += (vec_ptr)->elem_size)


static const size_t FOS_VEC_INIT_CAP = 16;

typedef struct {
    void   *data;
    size_t  size;
    size_t  capacity;
    size_t  elem_size;
} FOS_Vec;

typedef int (*FOS_Compare)(const void *, const void *);

FOS_Vec FOS_vec_new(size_t elem_size);
void FOS_vec_free(FOS_Vec *vec);
void FOS_vec_clear(FOS_Vec *vec);
bool FOS_vec_reserve(FOS_Vec *vec, size_t new_capacity);
bool FOS_vec_push(FOS_Vec *vec, const void *elem);
void *FOS_vec_at(FOS_Vec *vec, size_t index);
void *FOS_vec_data(FOS_Vec *vec);
bool FOS_vec_pop(FOS_Vec *vec, void *out_elem);
bool FOS_vec_copy(FOS_Vec *dst, const FOS_Vec *src);
bool FOS_vec_shrink_to_fit(FOS_Vec *vec);
bool FOS_vec_resize(FOS_Vec *vec, size_t new_size);
bool FOS_vec_insert(FOS_Vec *vec, size_t index, const void *elem);
bool FOS_vec_erase_range(FOS_Vec *vec, size_t start, size_t end);
bool FOS_vec_erase_at(FOS_Vec *vec, size_t i);
bool FOS_vec_erase_unordered(FOS_Vec *vec, size_t i);
bool FOS_vec_sort(FOS_Vec *vec, FOS_Compare cmp);

#endif
