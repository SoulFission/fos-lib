#include "FOS_HashMap.h"

uint64_t FOS_fnv1a(const void *data, size_t size)
{
    const unsigned char *bytes = (const unsigned char *)data;

    uint64_t hash = 1469598103934665603ULL;  // FNV offset basis

    for (size_t i = 0; i < size; ++i)
    {
        hash ^= (uint64_t)bytes[i];
        hash *= 1099511628211ULL;             // FNV prime
    }

    return hash;
}

bool FOS_key_eq(const void *a, const void *b, size_t size)
{
    if (size == 0)
        return true;

    if (a == NULL || b == NULL)
        return false;

    return !memcmp(a, b, size);
}

FOS_HashMap FOS_hashmap_new(size_t key_size, size_t value_size, FOS_HashFn hash_fn, FOS_KeyEqFn key_eq_fn)
{
    if (key_size == 0 || value_size == 0 || hash_fn == NULL || key_eq_fn == NULL)
        return (FOS_HashMap) { 0 };

    FOS_HashMap hmap = { 0 };

    hmap.slots = calloc(FOS_HASHMAP_DEFAULT_CAP, sizeof(FOS_HashSlot));

    if (hmap.slots == NULL)
        return (FOS_HashMap) { 0 };
        
    hmap.capacity = FOS_HASHMAP_DEFAULT_CAP;
    hmap.size = 0;
    hmap.tombstones = 0;

    hmap.key_size = key_size;
    hmap.value_size = value_size;

    hmap.hash = hash_fn;
    hmap.eq = key_eq_fn;

    return hmap;
}

bool FOS_hashmap_put(FOS_HashMap *map, const void *key, const void *value)
{
    if (map == NULL || map->slots == NULL || key == NULL || value == NULL)
        return false;

    uint64_t hash = map->hash(key, map->key_size);
    size_t index = hash & (map->capacity - 1);

    size_t first_tombstone = SIZE_MAX;

    for (size_t probe = 0; probe < map->capacity; ++probe)
    {
        FOS_HashSlot *slot = &map->slots[index];

        if (slot->state == FOS_OCCUPIED)
        {
            if (slot->hash == hash && map->eq(slot->key, key, map->key_size))
            {
                memcpy(slot->value, value, map->value_size);
                return true;
            }
        }
        else if (slot->state == FOS_TOMBSTONE && first_tombstone == SIZE_MAX)
        {
            first_tombstone = index;
        }
        else if (slot->state == FOS_EMPTY)
        {
            if (first_tombstone != SIZE_MAX)
            {
                map->tombstones--;
                index = first_tombstone;
            }

            slot = &map->slots[index];
            slot->key = malloc(map->key_size);

            if (slot->key == NULL)
            {
                if (first_tombstone != SIZE_MAX)
                    map->tombstones++;

                return false;
            }   

            slot->value = malloc(map->value_size);

            if (slot->value == NULL)
            {
                free(slot->key);

                if (first_tombstone != SIZE_MAX)
                    map->tombstones++;

                return false;
            }

            memcpy(slot->key, key, map->key_size);
            memcpy(slot->value, value, map->value_size);

            slot->state = FOS_OCCUPIED;
            slot->hash = hash;

            map->size++;

            if ((double)(map->size + map->tombstones) / map->capacity > FOS_THRESHOLD)
                if (!FOS_hashmap_resize(map))
                    return false;

            return true;
        }

        index = (index + 1) & (map->capacity - 1);
    }

    if (first_tombstone != SIZE_MAX)
    {
        FOS_HashSlot *slot = &map->slots[first_tombstone];

        slot->key = malloc(map->key_size);

        if (slot->key == NULL)
            return false;

        slot->value = malloc(map->value_size);

        if (slot->value == NULL)
        {
            free(slot->key);
            return false;
        }

        memcpy(slot->key, key, map->key_size);
        memcpy(slot->value, value, map->value_size);

        slot->state = FOS_OCCUPIED;
        slot->hash = hash;

        map->size++;
        map->tombstones--;

        if ((double)(map->size + map->tombstones) / map->capacity > FOS_THRESHOLD)
            if (!FOS_hashmap_resize(map))
                return false;

        return true;
    }

    return false;
}

bool FOS_hashmap_get(const FOS_HashMap *map, const void *key, void *out_value)
{
    if (map == NULL || map->slots == NULL || key == NULL || out_value == NULL || map->size == 0)
        return false;
    
    uint64_t hash = map->hash(key, map->key_size);
    size_t index = hash & (map->capacity - 1);

    for (size_t probe = 0; probe < map->capacity; ++probe)
    {
        FOS_HashSlot *slot = &map->slots[index];

        if (slot->state == FOS_EMPTY)
            return false;

        if (slot->state == FOS_OCCUPIED)
        {
            if (slot->hash == hash && map->eq(slot->key, key, map->key_size))
            {
                memcpy(out_value, slot->value, map->value_size);
                return true;
            }
        }

        index = (index + 1) & (map->capacity - 1);
    }

    return false;
}

void FOS_hashmap_free(FOS_HashMap *map)
{
    if (map == NULL || map->slots == NULL)
        return;

    for (size_t i = 0; i < map->capacity; ++i)
    {
        FOS_HashSlot *slot = &map->slots[i];

        if (slot->state == FOS_OCCUPIED)
        {
            free(slot->key);
            free(slot->value);
        }
    }

    free(map->slots);

    map->slots = NULL;
    map->capacity = 0;
    map->size = 0;
    map->tombstones = 0;
}

bool FOS_hashmap_is_valid(const FOS_HashMap *map)
{
    return (map != NULL && map->slots != NULL && map->capacity > 0 && map->hash != NULL && map->eq != NULL);
}

bool FOS_hashmap_remove(FOS_HashMap *map, const void *key)
{
    if (map == NULL || map->slots == NULL || key == NULL)
        return false;

    uint64_t hash = map->hash(key, map->key_size);
    size_t index = hash & (map->capacity - 1);

    for (size_t probe = 0; probe < map->capacity; ++probe)
    {
        FOS_HashSlot *slot = &map->slots[index];

        if (slot->state == FOS_EMPTY)
            return false;

        if (slot->state == FOS_OCCUPIED)
        {
            if (slot->hash == hash && map->eq(slot->key, key, map->key_size))
            {
                free(slot->key);
                free(slot->value);
                
                slot->state = FOS_TOMBSTONE;

                map->size--;
                map->tombstones++;

                return true;
            }
        }

        index = (index + 1) & (map->capacity - 1);
    }

    return false;
}

bool FOS_hashmap_resize(FOS_HashMap *map)
{
    if (map == NULL || map->slots == NULL)
        return false;

    size_t new_capacity = map->capacity * 2;

    if (new_capacity < map->capacity) 
        return false; // overflow protection

    FOS_HashSlot *slots = calloc(new_capacity, sizeof(FOS_HashSlot)); // all zeros, no need to assign FOS_EMPTY to every element

    if (slots == NULL)
        return false;

    void **allocs = calloc(map->size * 2, sizeof(void *));
    size_t to_free = 0;

    for (size_t i = 0; i < map->capacity; ++i)
    {
        if (map->slots[i].state == FOS_OCCUPIED)
        {
            uint64_t hash = map->hash(map->slots[i].key, map->key_size);
            size_t index = hash & (new_capacity - 1);

            while (slots[index].state == FOS_OCCUPIED)
                index = (index + 1) & (new_capacity - 1);

            FOS_HashSlot *slot = &map->slots[i];
            FOS_HashSlot *new_slot = &slots[index];

            new_slot->key = malloc(map->key_size);

            if (new_slot->key == NULL)
                goto cleanup;

            allocs[to_free++] = new_slot->key;

            new_slot->value = malloc(map->value_size);

            if (new_slot->value == NULL)
                goto cleanup;

            allocs[to_free++] = new_slot->value;

            memcpy(new_slot->key, slot->key, map->key_size);
            memcpy(new_slot->value, slot->value, map->value_size);
            
            new_slot->state = FOS_OCCUPIED;
            new_slot->hash = hash;
        }
    }

    free(map->slots);

    map->slots = slots;
    map->capacity = new_capacity;
    map->tombstones = 0;

    // size stays the same
    free(allocs);

    return true;

cleanup:
    for (size_t i = 0; i < to_free; ++i)
        free(allocs[i]);
    free(allocs);

    return false;
}
