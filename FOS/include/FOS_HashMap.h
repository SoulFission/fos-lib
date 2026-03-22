#ifndef FOS_HASHMAP_H
#define FOS_HASHMAP_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef uint64_t (*FOS_HashFn)(const void *data, size_t size);
typedef bool (*FOS_KeyEqFn)(const void *a, const void *b, size_t key_size);

static const size_t FOS_HASHMAP_DEFAULT_CAP = 128;
static const double FOS_THRESHOLD = 0.75;

typedef enum {
    FOS_EMPTY,
    FOS_OCCUPIED,
    FOS_TOMBSTONE
} FOS_SlotState;

typedef struct {
    void *key;
    void *value;
    uint64_t hash;
    FOS_SlotState state;
} FOS_HashSlot;

typedef struct {
    FOS_HashSlot *slots;
    size_t capacity;
    size_t size;
    size_t tombstones;

    size_t key_size;
    size_t value_size;

    FOS_HashFn hash;
    FOS_KeyEqFn eq;
} FOS_HashMap;

uint64_t FOS_fnv1a(const void *data, size_t size);
bool FOS_key_eq(const void *a, const void *b, size_t size);
FOS_HashMap FOS_hashmap_new(size_t key_size, size_t value_size, FOS_HashFn hash_fn, FOS_KeyEqFn key_eq_fn);
bool FOS_hashmap_put(FOS_HashMap *map, const void *key, const void *value);
bool FOS_hashmap_get(const FOS_HashMap *map, const void *key, void *out_value);
void FOS_hashmap_free(FOS_HashMap *map);
bool FOS_hashmap_is_valid(const FOS_HashMap *map);
bool FOS_hashmap_remove(FOS_HashMap *map, const void *key);
bool FOS_hashmap_resize(FOS_HashMap *map);

#endif
