#include "FOS_HashMap.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*
 * --------------------------------------------------------------------------
 * Test infrastructure
 * --------------------------------------------------------------------------
 */

#define CHECK(expr)                                                        \
    do                                                                      \
    {                                                                       \
        if (!(expr))                                                        \
        {                                                                   \
            fprintf(                                                        \
                stderr,                                                     \
                "CHECK FAILED: %s\n  at %s:%d\n",                           \
                #expr, __FILE__, __LINE__                                  \
            );                                                              \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
    } while (0)

static void expect_valid(const FOS_HashMap *map)
{
    CHECK(map != NULL);
    CHECK(FOS_hashmap_is_valid(map));
}

static void expect_invalid(const FOS_HashMap *map)
{
    CHECK(map != NULL);
    CHECK(!FOS_hashmap_is_valid(map));
}

static FOS_HashMap new_map(size_t key_size, size_t value_size)
{
    FOS_HashMap map =
        FOS_hashmap_new(
            key_size,
            value_size,
            FOS_fnv1a,
            FOS_key_eq
        );

    expect_valid(&map);

    return map;
}

static void expect_value(
    const FOS_HashMap *map,
    const void *key,
    const void *expected
)
{
    unsigned char *actual = malloc(map->value_size);
    CHECK(actual != NULL);

    CHECK(FOS_hashmap_get(map, key, actual));
    CHECK(memcmp(actual, expected, map->value_size) == 0);

    free(actual);
}

static void expect_missing(const FOS_HashMap *map, const void *key)
{
    CHECK(!FOS_hashmap_get(map, key, NULL));
    CHECK(!FOS_hashmap_contains(map, key));
}

/*
 * --------------------------------------------------------------------------
 * Hash functions used by tests
 * --------------------------------------------------------------------------
 */

static uint64_t constant_hash(const void *data, size_t size)
{
    (void)data;
    (void)size;

    return UINT64_C(0x123456789ABCDEF0);
}

/*
 * Case-insensitive hash.

 * This deliberately matches case_insensitive_eq() below. Equal keys MUST
 * produce equal hashes for a hash table to work correctly.
 */
static uint64_t case_insensitive_hash(const void *data, size_t size)
{
    const unsigned char *bytes = data;
    uint64_t hash = UINT64_C(14695981039346656037);

    for (size_t i = 0; i < size; ++i)
    {
        unsigned char c = bytes[i];

        if (c >= 'A' && c <= 'Z')
            c = (unsigned char)(c + ('a' - 'A'));

        hash ^= (uint64_t)c;
        hash *= UINT64_C(1099511628211);
    }

    return hash;
}

static bool case_insensitive_eq(
    const void *a,
    const void *b,
    size_t size
)
{
    const unsigned char *aa = a;
    const unsigned char *bb = b;

    if (size == 0)
        return true;

    if (a == NULL || b == NULL)
        return false;

    for (size_t i = 0; i < size; ++i)
    {
        unsigned char ca = aa[i];
        unsigned char cb = bb[i];

        if (ca >= 'A' && ca <= 'Z')
            ca = (unsigned char)(ca + ('a' - 'A'));

        if (cb >= 'A' && cb <= 'Z')
            cb = (unsigned char)(cb + ('a' - 'A'));

        if (ca != cb)
            return false;
    }

    return true;
}

/*
 * --------------------------------------------------------------------------
 * FNV-1a tests
 * --------------------------------------------------------------------------
 */

static void test_fnv1a(void)
{
    /*
     * FNV-1a known vectors.
     */
    static const unsigned char empty[] = "";
    static const unsigned char a[] = "a";
    static const unsigned char hello[] = "hello";

    CHECK(
        FOS_fnv1a(empty, 0) ==
        UINT64_C(14695981039346656037)
    );

    CHECK(
        FOS_fnv1a(a, 1) ==
        UINT64_C(12638187200555641996)
    );

    CHECK(
        FOS_fnv1a(hello, 5) ==
        UINT64_C(11831194018420276491)
    );
}

/*
 * --------------------------------------------------------------------------
 * Equality tests
 * --------------------------------------------------------------------------
 */

static void test_key_eq(void)
{
    int a = 42;
    int b = 42;
    int c = 43;

    CHECK(FOS_key_eq(&a, &b, sizeof(a)));
    CHECK(!FOS_key_eq(&a, &c, sizeof(a)));

    CHECK(FOS_key_eq(NULL, NULL, 0));
    CHECK(FOS_key_eq(&a, NULL, 0));
    CHECK(FOS_key_eq(NULL, &a, 0));

    CHECK(!FOS_key_eq(NULL, &a, sizeof(a)));
    CHECK(!FOS_key_eq(&a, NULL, sizeof(a)));

    {
        const unsigned char x[] = { 0x00, 0x01, 0x02, 0xFF };
        const unsigned char y[] = { 0x00, 0x01, 0x02, 0xFF };
        const unsigned char z[] = { 0x00, 0x01, 0x03, 0xFF };

        CHECK(FOS_key_eq(x, y, sizeof(x)));
        CHECK(!FOS_key_eq(x, z, sizeof(x)));
    }
}

/*
 * --------------------------------------------------------------------------
 * Construction
 * --------------------------------------------------------------------------
 */

static void test_construction(void)
{
    FOS_HashMap map =
        FOS_hashmap_new(
            sizeof(int),
            sizeof(int),
            FOS_fnv1a,
            FOS_key_eq
        );

    expect_valid(&map);

    CHECK(map.slots != NULL);
    CHECK(map.capacity == FOS_HASHMAP_DEFAULT_CAP);
    CHECK(map.size == 0);
    CHECK(map.tombstones == 0);
    CHECK(map.key_size == sizeof(int));
    CHECK(map.value_size == sizeof(int));
    CHECK(map.hash == FOS_fnv1a);
    CHECK(map.eq == FOS_key_eq);

    FOS_hashmap_free(&map);
}

/*
 * NULL callbacks are NOT invalid.

 * The current constructor substitutes the default functions.
 */
static void test_default_callbacks(void)
{
    FOS_HashMap map =
        FOS_hashmap_new(
            sizeof(int),
            sizeof(int),
            NULL,
            NULL
        );

    expect_valid(&map);

    CHECK(map.hash == FOS_fnv1a);
    CHECK(map.eq == FOS_key_eq);

    {
        int key = 123;
        int value = 456;
        int result = 0;

        CHECK(FOS_hashmap_put(&map, &key, &value));
        CHECK(FOS_hashmap_get(&map, &key, &result));
        CHECK(result == value);
    }

    FOS_hashmap_free(&map);
}

static void test_invalid_construction(void)
{
    FOS_HashMap a =
        FOS_hashmap_new(
            0,
            sizeof(int),
            FOS_fnv1a,
            FOS_key_eq
        );

    FOS_HashMap b =
        FOS_hashmap_new(
            sizeof(int),
            0,
            FOS_fnv1a,
            FOS_key_eq
        );

    /*
     * NULL callbacks are valid under the current constructor contract.
     */
    FOS_HashMap c =
        FOS_hashmap_new(
            sizeof(int),
            sizeof(int),
            NULL,
            FOS_key_eq
        );

    FOS_HashMap d =
        FOS_hashmap_new(
            sizeof(int),
            sizeof(int),
            FOS_fnv1a,
            NULL
        );

    FOS_HashMap e =
        FOS_hashmap_new(
            sizeof(int),
            sizeof(int),
            NULL,
            NULL
        );

    expect_invalid(&a);
    expect_invalid(&b);

    expect_valid(&c);
    expect_valid(&d);
    expect_valid(&e);

    CHECK(c.hash == FOS_fnv1a);
    CHECK(c.eq == FOS_key_eq);

    CHECK(d.hash == FOS_fnv1a);
    CHECK(d.eq == FOS_key_eq);

    CHECK(e.hash == FOS_fnv1a);
    CHECK(e.eq == FOS_key_eq);

    FOS_hashmap_free(&a);
    FOS_hashmap_free(&b);
    FOS_hashmap_free(&c);
    FOS_hashmap_free(&d);
    FOS_hashmap_free(&e);
}

/*
 * --------------------------------------------------------------------------
 * Basic integer operations
 * --------------------------------------------------------------------------
 */

static void test_integer_keys(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    for (int i = 0; i < 100; ++i)
    {
        int value = i * 10;

        CHECK(FOS_hashmap_put(&map, &i, &value));
    }

    CHECK(map.size == 100);

    for (int i = 0; i < 100; ++i)
    {
        int expected = i * 10;
        expect_value(&map, &i, &expected);
        CHECK(FOS_hashmap_contains(&map, &i));
    }

    {
        int missing = 1000;
        expect_missing(&map, &missing);
    }

    FOS_hashmap_free(&map);
}

static void test_update_existing_key(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    int key = 42;
    int value1 = 100;
    int value2 = 200;

    CHECK(FOS_hashmap_put(&map, &key, &value1));
    CHECK(map.size == 1);

    CHECK(FOS_hashmap_put(&map, &key, &value2));
    CHECK(map.size == 1);

    expect_value(&map, &key, &value2);

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * String keys
 * --------------------------------------------------------------------------
 */

typedef struct
{
    char text[32];
} Key;

static void test_string_keys(void)
{
    FOS_HashMap map = new_map(sizeof(Key), sizeof(int));

    Key apple = { "apple" };
    Key banana = { "banana" };
    Key cherry = { "cherry" };

    int a = 1;
    int b = 2;
    int c = 3;

    CHECK(FOS_hashmap_put(&map, &apple, &a));
    CHECK(FOS_hashmap_put(&map, &banana, &b));
    CHECK(FOS_hashmap_put(&map, &cherry, &c));

    expect_value(&map, &apple, &a);
    expect_value(&map, &banana, &b);
    expect_value(&map, &cherry, &c);

    {
        Key missing = { "orange" };
        expect_missing(&map, &missing);
    }

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Binary keys / values
 * --------------------------------------------------------------------------
 */

static void test_binary_data(void)
{
    FOS_HashMap map = new_map(8, 16);

    unsigned char key[] =
    {
        0x00, 0x01, 0x02, 0x00,
        0xFE, 0xFF, 0x10, 0x20
    };

    unsigned char value[] =
    {
        0x00, 0xFF, 0x00, 0x01,
        0x02, 0x03, 0x04, 0x05,
        0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x0C, 0x0D
    };

    unsigned char result[sizeof(value)];

    CHECK(FOS_hashmap_put(&map, key, value));
    CHECK(FOS_hashmap_get(&map, key, result));
    CHECK(memcmp(result, value, sizeof(value)) == 0);

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Pointer-sized keys

 * The map treats keys as raw bytes. This test therefore stores the pointer
 * representation itself as the key bytes.
 * --------------------------------------------------------------------------
 */

static void test_pointer_data(void)
{
    FOS_HashMap map = new_map(sizeof(void *), sizeof(int));

    int object1 = 10;
    int object2 = 20;

    void *key1 = &object1;
    void *key2 = &object2;

    int value1 = 100;
    int value2 = 200;

    CHECK(FOS_hashmap_put(&map, &key1, &value1));
    CHECK(FOS_hashmap_put(&map, &key2, &value2));

    expect_value(&map, &key1, &value1);
    expect_value(&map, &key2, &value2);

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * NULL output argument

 * get() deliberately permits out_value == NULL because contains() uses it.
 */

static void test_null_output_value(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    int key = 123;
    int value = 456;

    CHECK(FOS_hashmap_put(&map, &key, &value));

    CHECK(FOS_hashmap_get(&map, &key, NULL));
    CHECK(FOS_hashmap_contains(&map, &key));

    {
        int missing = 999;

        CHECK(!FOS_hashmap_get(&map, &missing, NULL));
        CHECK(!FOS_hashmap_contains(&map, &missing));
    }

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Collision handling
 * --------------------------------------------------------------------------
 */

static void test_collisions(void)
{
    FOS_HashMap map =
        FOS_hashmap_new(
            sizeof(int),
            sizeof(int),
            constant_hash,
            FOS_key_eq
        );

    expect_valid(&map);

    for (int i = 0; i < 32; ++i)
    {
        int value = i * 100;

        CHECK(FOS_hashmap_put(&map, &i, &value));
    }

    CHECK(map.size == 32);

    for (int i = 0; i < 32; ++i)
    {
        int expected = i * 100;
        expect_value(&map, &i, &expected);
    }

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Custom equality + compatible custom hash
 * --------------------------------------------------------------------------
 */

static void test_custom_hash_and_equality(void)
{
    FOS_HashMap map =
        FOS_hashmap_new(
            16,
            sizeof(int),
            case_insensitive_hash,
            case_insensitive_eq
        );

    expect_valid(&map);

    char key1[16] = { 0 };
    char key2[16] = { 0 };

    memcpy(key1, "Hello", 5);
    memcpy(key2, "hello", 5);

    int value1 = 123;
    int value2 = 456;

    CHECK(FOS_hashmap_put(&map, key1, &value1));
    CHECK(map.size == 1);

    /*
     * key1 and key2 are equal under case_insensitive_eq(), and their
     * compatible hashes are therefore equal. This must update the value
     * rather than insert another entry.
     */
    CHECK(FOS_hashmap_put(&map, key2, &value2));
    CHECK(map.size == 1);

    expect_value(&map, key1, &value2);
    expect_value(&map, key2, &value2);

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Removal
 * --------------------------------------------------------------------------
 */

static void test_remove(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    int key1 = 1;
    int key2 = 2;
    int key3 = 3;

    int value1 = 10;
    int value2 = 20;
    int value3 = 30;

    CHECK(FOS_hashmap_put(&map, &key1, &value1));
    CHECK(FOS_hashmap_put(&map, &key2, &value2));
    CHECK(FOS_hashmap_put(&map, &key3, &value3));

    CHECK(map.size == 3);
    CHECK(map.tombstones == 0);

    CHECK(FOS_hashmap_remove(&map, &key2));

    CHECK(map.size == 2);
    CHECK(map.tombstones == 1);

    CHECK(!FOS_hashmap_contains(&map, &key2));
    expect_value(&map, &key1, &value1);
    expect_value(&map, &key3, &value3);

    CHECK(!FOS_hashmap_remove(&map, &key2));

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Tombstone probing
 * --------------------------------------------------------------------------
 */

static void test_tombstones_preserve_probe_chain(void)
{
    FOS_HashMap map =
        FOS_hashmap_new(
            sizeof(int),
            sizeof(int),
            constant_hash,
            FOS_key_eq
        );

    expect_valid(&map);

    int keys[8];
    int values[8];

    for (int i = 0; i < 8; ++i)
    {
        keys[i] = i;
        values[i] = i + 1000;

        CHECK(FOS_hashmap_put(&map, &keys[i], &values[i]));
    }

    CHECK(map.size == 8);

    /*
     * Remove entries from the middle of the collision chain.
     */
    CHECK(FOS_hashmap_remove(&map, &keys[2]));
    CHECK(FOS_hashmap_remove(&map, &keys[4]));
    CHECK(FOS_hashmap_remove(&map, &keys[6]));

    CHECK(map.size == 5);
    CHECK(map.tombstones == 3);

    /*
     * The remaining entries must still be discoverable through tombstones.
     */
    CHECK(FOS_hashmap_contains(&map, &keys[0]));
    CHECK(FOS_hashmap_contains(&map, &keys[1]));
    CHECK(FOS_hashmap_contains(&map, &keys[3]));
    CHECK(FOS_hashmap_contains(&map, &keys[5]));
    CHECK(FOS_hashmap_contains(&map, &keys[7]));

    FOS_hashmap_free(&map);
}

static void test_tombstone_reuse(void)
{
    FOS_HashMap map =
        FOS_hashmap_new(
            sizeof(int),
            sizeof(int),
            constant_hash,
            FOS_key_eq
        );

    expect_valid(&map);

    int key1 = 1;
    int key2 = 2;
    int key3 = 3;

    int value1 = 10;
    int value2 = 20;
    int value3 = 30;

    CHECK(FOS_hashmap_put(&map, &key1, &value1));
    CHECK(FOS_hashmap_put(&map, &key2, &value2));

    CHECK(FOS_hashmap_remove(&map, &key1));

    CHECK(map.size == 1);
    CHECK(map.tombstones == 1);

    CHECK(FOS_hashmap_put(&map, &key3, &value3));

    CHECK(map.size == 2);
    CHECK(map.tombstones == 0);

    expect_value(&map, &key2, &value2);
    expect_value(&map, &key3, &value3);

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Resizing
 * --------------------------------------------------------------------------
 */

static void test_explicit_resize(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    size_t old_capacity = map.capacity;

    CHECK(FOS_hashmap_resize(&map));

    CHECK(map.capacity == old_capacity * 2);
    CHECK(map.size == 0);
    CHECK(map.tombstones == 0);

    FOS_hashmap_free(&map);
}

static void test_resize_preserves_entries(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    for (int i = 0; i < 200; ++i)
    {
        int value = i * 7;

        CHECK(FOS_hashmap_put(&map, &i, &value));
    }

    CHECK(map.size == 200);

    for (int i = 0; i < 200; ++i)
    {
        int expected = i * 7;

        expect_value(&map, &i, &expected);
    }

    CHECK(map.capacity >= FOS_HASHMAP_DEFAULT_CAP);

    FOS_hashmap_free(&map);
}

static void test_resize_with_tombstones(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    for (int i = 0; i < 100; ++i)
    {
        int value = i * 10;

        CHECK(FOS_hashmap_put(&map, &i, &value));
    }

    for (int i = 0; i < 50; ++i)
        CHECK(FOS_hashmap_remove(&map, &i));

    CHECK(map.size == 50);
    CHECK(map.tombstones == 50);

    CHECK(FOS_hashmap_resize(&map));

    CHECK(map.size == 50);
    CHECK(map.tombstones == 0);

    for (int i = 0; i < 50; ++i)
    {
        CHECK(!FOS_hashmap_contains(&map, &i));
    }

    for (int i = 50; i < 100; ++i)
    {
        int expected = i * 10;

        expect_value(&map, &i, &expected);
    }

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Larger mutation sequence
 * --------------------------------------------------------------------------
 */

static void test_repeated_mutation(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    /*
     * Insert.
     */
    for (int i = 0; i < 500; ++i)
    {
        int value = i * 2;

        CHECK(FOS_hashmap_put(&map, &i, &value));
    }

    CHECK(map.size == 500);

    /*
     * Remove every third key.
     */
    for (int i = 0; i < 500; i += 3)
        CHECK(FOS_hashmap_remove(&map, &i));

    /*
     * Check remaining entries.
     */
    for (int i = 0; i < 500; ++i)
    {
        if (i % 3 == 0)
        {
            CHECK(!FOS_hashmap_contains(&map, &i));
        }
        else
        {
            int expected = i * 2;
            expect_value(&map, &i, &expected);
        }
    }

    /*
     * Reinsert removed keys with different values.
     */
    for (int i = 0; i < 500; i += 3)
    {
        int value = i * 5;

        CHECK(FOS_hashmap_put(&map, &i, &value));
    }

    CHECK(map.size == 500);

    for (int i = 0; i < 500; ++i)
    {
        int expected;

        if (i % 3 == 0)
            expected = i * 5;
        else
            expected = i * 2;

        expect_value(&map, &i, &expected);
    }

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Argument validation
 * --------------------------------------------------------------------------
 */

static void test_null_arguments(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    int key = 1;
    int value = 2;
    int result = 0;

    CHECK(!FOS_hashmap_put(NULL, &key, &value));
    CHECK(!FOS_hashmap_put(&map, NULL, &value));
    CHECK(!FOS_hashmap_put(&map, &key, NULL));

    CHECK(!FOS_hashmap_get(NULL, &key, &result));
    CHECK(!FOS_hashmap_get(&map, NULL, &result));

    CHECK(!FOS_hashmap_contains(NULL, &key));
    CHECK(!FOS_hashmap_contains(&map, NULL));

    CHECK(!FOS_hashmap_remove(NULL, &key));
    CHECK(!FOS_hashmap_remove(&map, NULL));

    CHECK(!FOS_hashmap_resize(NULL));

    CHECK(!FOS_hashmap_is_valid(NULL));

    /*
     * get(..., NULL) is intentionally NOT tested as invalid.
     */
    CHECK(FOS_hashmap_put(&map, &key, &value));
    CHECK(FOS_hashmap_get(&map, &key, NULL));

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Empty map
 * --------------------------------------------------------------------------
 */

static void test_empty_map(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    int key = 123;
    int value = 0;

    CHECK(map.size == 0);
    CHECK(map.tombstones == 0);

    CHECK(!FOS_hashmap_get(&map, &key, &value));
    CHECK(!FOS_hashmap_get(&map, &key, NULL));
    CHECK(!FOS_hashmap_contains(&map, &key));
    CHECK(!FOS_hashmap_remove(&map, &key));

    FOS_hashmap_free(&map);
}

/*
 * --------------------------------------------------------------------------
 * Freeing
 * --------------------------------------------------------------------------
 */

static void test_free(void)
{
    FOS_HashMap map = new_map(sizeof(int), sizeof(int));

    for (int i = 0; i < 10; ++i)
    {
        int value = i * 2;

        CHECK(FOS_hashmap_put(&map, &i, &value));
    }

    FOS_hashmap_free(&map);

    CHECK(map.slots == NULL);
    CHECK(map.capacity == 0);
    CHECK(map.size == 0);
    CHECK(map.tombstones == 0);

    CHECK(!FOS_hashmap_is_valid(&map));

    /*
     * Double-free is harmless under the current implementation.
     */
    FOS_hashmap_free(&map);

    CHECK(map.slots == NULL);
    CHECK(map.capacity == 0);
    CHECK(map.size == 0);
    CHECK(map.tombstones == 0);
}

static void test_free_null(void)
{
    FOS_hashmap_free(NULL);
}

/*
 * --------------------------------------------------------------------------
 * Main
 * --------------------------------------------------------------------------
 */

int main(void)
{
    printf("Testing FOS_HashMap...\n");

    test_fnv1a();
    test_key_eq();

    test_construction();
    test_default_callbacks();
    test_invalid_construction();

    test_empty_map();
    test_integer_keys();
    test_update_existing_key();
    test_string_keys();
    test_binary_data();
    test_pointer_data();

    test_null_output_value();
    test_null_arguments();

    test_collisions();
    test_custom_hash_and_equality();

    test_remove();
    test_tombstones_preserve_probe_chain();
    test_tombstone_reuse();

    test_explicit_resize();
    test_resize_preserves_entries();
    test_resize_with_tombstones();

    test_repeated_mutation();

    test_free();
    test_free_null();

    printf("All FOS_HashMap tests passed.\n");

    return EXIT_SUCCESS;
}
