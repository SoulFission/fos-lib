#ifndef FOS_SET_H
#define FOS_SET_H

#include <stdint.h>
#include "FOS_HashMap.h"

typedef struct {
    FOS_HashMap hash_map;
} FOS_Set;

static const uint8_t FOS_UNUSED = 1;

FOS_Set FOS_set_new(size_t elem_size);
bool FOS_set_contains(const FOS_Set *set, const void *elem);
bool FOS_set_add(FOS_Set *set, const void *elem);
bool FOS_set_remove(FOS_Set *set, const void *elem);
size_t FOS_set_get_size(const FOS_Set *set);
void FOS_set_iterate(const FOS_Set *set, void (*callback)(const void *elem, void *user), void *user);
void FOS_set_free(FOS_Set *set);

#endif
