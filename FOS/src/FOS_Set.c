#include "FOS_Set.h"
#include <string.h>

FOS_Set FOS_set_new(size_t elem_size)
{
    if (elem_size == 0)
        return (FOS_Set) { 0 };

    FOS_Set new_set = { 0 };

    new_set.hash_map = FOS_hashmap_new(elem_size, sizeof(uint8_t), FOS_fnv1a, FOS_key_eq);

    return new_set;
}

bool FOS_set_contains(const FOS_Set *set, const void *elem)
{
    if (set == NULL || elem == NULL)
        return false;

    return FOS_hashmap_contains(&set->hash_map, elem);
}

bool FOS_set_add(FOS_Set *set, const void *elem)
{
    if (set == NULL || elem == NULL)
        return 0;

    if (FOS_set_contains(set, elem))
        return 0;

    // double lookup is required with the current implementation
    return FOS_hashmap_put(&set->hash_map, elem, &FOS_UNUSED);
}

bool FOS_set_remove(FOS_Set *set, const void *elem)
{
    if (set == NULL || elem == NULL)
        return false;

    return FOS_hashmap_remove(&set->hash_map, elem);
}

size_t FOS_set_get_size(const FOS_Set *set)
{
    if (set == NULL)
        return 0;

    return set->hash_map.size;
}

void FOS_set_iterate(const FOS_Set *set, void (*callback)(const void *elem, void *user), void *user)
{
    for (size_t i = 0; i < set->hash_map.capacity; ++i)
    {
        FOS_HashSlot *slot = &set->hash_map.slots[i];

        if (slot->state == FOS_OCCUPIED)
            callback(slot->key, user);
    }
}

void FOS_set_free(FOS_Set *set)
{
    if (set == NULL)
        return;

    FOS_hashmap_free(&set->hash_map);

    memset(set, 0, sizeof(*set));
}
