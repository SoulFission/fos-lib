#include "FOS_Set.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Test helpers
 * -------------------------------------------------------------------------- */

static void assert_set_empty(const FOS_Set *set)
{
    assert(set != NULL);
    assert(FOS_set_get_size(set) == 0);
}

static void assert_set_size(const FOS_Set *set, size_t expected)
{
    assert(set != NULL);
    assert(FOS_set_get_size(set) == expected);
}

/* --------------------------------------------------------------------------
 * Iteration helpers
 * -------------------------------------------------------------------------- */

typedef struct {
    size_t count;
    int sum;
} IterateIntContext;

static void collect_int(const void *elem, void *user)
{
    const int *value = elem;
    IterateIntContext *ctx = user;

    assert(value != NULL);
    assert(ctx != NULL);

    ++ctx->count;
    ctx->sum += *value;
}

/* --------------------------------------------------------------------------
 * Construction
 * -------------------------------------------------------------------------- */

static void test_new(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    assert(FOS_set_get_size(&set) == 0);
    assert(FOS_set_contains(&set, &(int){42}) == false);

    FOS_set_free(&set);
}

static void test_new_invalid(void)
{
    FOS_Set set = FOS_set_new(0);

    assert(FOS_set_get_size(&set) == 0);
    assert(FOS_set_contains(&set, &(int){42}) == false);

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Add / contains
 * -------------------------------------------------------------------------- */

static void test_add_and_contains(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    int a = 10;
    int b = 20;
    int c = 30;

    assert(FOS_set_add(&set, &a) == true);
    assert(FOS_set_add(&set, &b) == true);
    assert(FOS_set_add(&set, &c) == true);

    assert_set_size(&set, 3);

    assert(FOS_set_contains(&set, &a) == true);
    assert(FOS_set_contains(&set, &b) == true);
    assert(FOS_set_contains(&set, &c) == true);

    int missing = 40;

    assert(FOS_set_contains(&set, &missing) == false);

    FOS_set_free(&set);
}

static void test_duplicate_add(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    int value = 123;

    assert(FOS_set_add(&set, &value) == true);
    assert(FOS_set_add(&set, &value) == false);

    assert_set_size(&set, 1);
    assert(FOS_set_contains(&set, &value) == true);

    FOS_set_free(&set);
}

static void test_duplicate_by_value(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    int a = 42;
    int b = 42;

    assert(&a != &b);

    assert(FOS_set_add(&set, &a) == true);
    assert(FOS_set_add(&set, &b) == false);

    assert_set_size(&set, 1);
    assert(FOS_set_contains(&set, &a) == true);
    assert(FOS_set_contains(&set, &b) == true);

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Remove
 * -------------------------------------------------------------------------- */

static void test_remove(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    int a = 10;
    int b = 20;
    int c = 30;

    assert(FOS_set_add(&set, &a));
    assert(FOS_set_add(&set, &b));
    assert(FOS_set_add(&set, &c));

    assert_set_size(&set, 3);

    assert(FOS_set_remove(&set, &b) == true);

    assert_set_size(&set, 2);
    assert(FOS_set_contains(&set, &a) == true);
    assert(FOS_set_contains(&set, &b) == false);
    assert(FOS_set_contains(&set, &c) == true);

    /* Removing it again must fail. */
    assert(FOS_set_remove(&set, &b) == false);
    assert_set_size(&set, 2);

    FOS_set_free(&set);
}

static void test_remove_and_readd(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    int value = 999;

    assert(FOS_set_add(&set, &value));
    assert(FOS_set_remove(&set, &value));
    assert(!FOS_set_contains(&set, &value));
    assert_set_size(&set, 0);

    assert(FOS_set_add(&set, &value));
    assert(FOS_set_contains(&set, &value));
    assert_set_size(&set, 1);

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Binary elements
 * -------------------------------------------------------------------------- */

typedef struct {
    uint8_t bytes[16];
} Blob;

static void test_binary_elements(void)
{
    FOS_Set set = FOS_set_new(sizeof(Blob));

    Blob a = {
        .bytes = {
            0x00, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b,
            0x0c, 0x0d, 0x0e, 0x0f
        }
    };

    Blob b = a;

    Blob c = {
        .bytes = {
            0xff, 0xff, 0xff, 0xff,
            0x00, 0x00, 0x00, 0x00,
            0x11, 0x22, 0x33, 0x44,
            0x55, 0x66, 0x77, 0x88
        }
    };

    assert(FOS_set_add(&set, &a));
    assert(!FOS_set_add(&set, &b));
    assert(FOS_set_add(&set, &c));

    assert_set_size(&set, 2);

    assert(FOS_set_contains(&set, &a));
    assert(FOS_set_contains(&set, &b));
    assert(FOS_set_contains(&set, &c));

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Pointer-valued elements
 *
 * The set stores the pointer value itself, because elem_size is sizeof(ptr).
 * -------------------------------------------------------------------------- */

static void test_pointer_elements(void)
{
    FOS_Set set = FOS_set_new(sizeof(void *));

    int a = 10;
    int b = 20;

    int *pa = &a;
    int *pb = &b;
    int *pa_again = &a;

    assert(FOS_set_add(&set, &pa));
    assert(FOS_set_add(&set, &pb));

    assert(!FOS_set_add(&set, &pa_again));

    assert(FOS_set_contains(&set, &pa));
    assert(FOS_set_contains(&set, &pb));

    assert_set_size(&set, 2);

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Collision testing
 * -------------------------------------------------------------------------- */

/*
 * FOS_set_new() uses FOS_fnv1a, so we can't inject a custom hash function
 * through the Set API. Instead, deliberately use many values to exercise
 * collisions/probing in the underlying HashMap.
 */
static void test_collisions_and_probing(void)
{
    FOS_Set set = FOS_set_new(sizeof(uint64_t));

    for (uint64_t i = 0; i < 1000; ++i)
        assert(FOS_set_add(&set, &i));

    assert_set_size(&set, 1000);

    for (uint64_t i = 0; i < 1000; ++i)
        assert(FOS_set_contains(&set, &i));

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Tombstones / repeated mutation
 * -------------------------------------------------------------------------- */

static void test_repeated_remove_and_add(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    for (int i = 0; i < 500; ++i)
        assert(FOS_set_add(&set, &i));

    assert_set_size(&set, 500);

    for (int i = 0; i < 500; i += 2)
        assert(FOS_set_remove(&set, &i));

    assert_set_size(&set, 250);

    for (int i = 0; i < 500; ++i)
    {
        if (i % 2 == 0)
            assert(!FOS_set_contains(&set, &i));
        else
            assert(FOS_set_contains(&set, &i));
    }

    /* Reinsert all removed elements. */
    for (int i = 0; i < 500; i += 2)
        assert(FOS_set_add(&set, &i));

    assert_set_size(&set, 500);

    for (int i = 0; i < 500; ++i)
        assert(FOS_set_contains(&set, &i));

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Iteration
 * -------------------------------------------------------------------------- */

static void test_iterate(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    int values[] = { 10, 20, 30, 40, 50 };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
        assert(FOS_set_add(&set, &values[i]));

    IterateIntContext ctx = { 0 };

    FOS_set_iterate(&set, collect_int, &ctx);

    assert(ctx.count == 5);
    assert(ctx.sum == 150);

    FOS_set_free(&set);
}

static void test_iterate_empty(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    IterateIntContext ctx = { 0 };

    FOS_set_iterate(&set, collect_int, &ctx);

    assert(ctx.count == 0);
    assert(ctx.sum == 0);

    FOS_set_free(&set);
}

static void test_iterate_after_removal(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    int values[] = { 1, 2, 3, 4, 5 };

    for (size_t i = 0; i < 5; ++i)
        assert(FOS_set_add(&set, &values[i]));

    assert(FOS_set_remove(&set, &values[1]));
    assert(FOS_set_remove(&set, &values[3]));

    IterateIntContext ctx = { 0 };

    FOS_set_iterate(&set, collect_int, &ctx);

    assert(ctx.count == 3);
    assert(ctx.sum == 1 + 3 + 5);

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Invalid arguments
 * -------------------------------------------------------------------------- */

static void test_invalid_arguments(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));
    int value = 42;

    assert(FOS_set_contains(NULL, &value) == false);
    assert(FOS_set_contains(&set, NULL) == false);

    assert(FOS_set_add(NULL, &value) == false);
    assert(FOS_set_add(&set, NULL) == false);

    assert(FOS_set_remove(NULL, &value) == false);
    assert(FOS_set_remove(&set, NULL) == false);

    assert(FOS_set_get_size(NULL) == 0);

    FOS_set_iterate(NULL, collect_int, NULL);
    FOS_set_iterate(&set, NULL, NULL);

    FOS_set_free(NULL);

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Large workload / resize
 * -------------------------------------------------------------------------- */

static void test_large_set(void)
{
    FOS_Set set = FOS_set_new(sizeof(size_t));

    enum { COUNT = 10000 };

    for (size_t i = 0; i < COUNT; ++i)
        assert(FOS_set_add(&set, &i));

    assert_set_size(&set, COUNT);

    for (size_t i = 0; i < COUNT; ++i)
        assert(FOS_set_contains(&set, &i));

    /* Every duplicate insertion must fail. */
    for (size_t i = 0; i < COUNT; ++i)
        assert(!FOS_set_add(&set, &i));

    assert_set_size(&set, COUNT);

    /* Remove half. */
    for (size_t i = 0; i < COUNT; i += 2)
        assert(FOS_set_remove(&set, &i));

    assert_set_size(&set, COUNT / 2);

    /* Remaining half must still be present. */
    for (size_t i = 1; i < COUNT; i += 2)
        assert(FOS_set_contains(&set, &i));

    /* Removed half must be absent. */
    for (size_t i = 0; i < COUNT; i += 2)
        assert(!FOS_set_contains(&set, &i));

    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Free / repeated free
 * -------------------------------------------------------------------------- */

static void test_free(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    int values[] = { 1, 2, 3 };

    for (size_t i = 0; i < 3; ++i)
        assert(FOS_set_add(&set, &values[i]));

    FOS_set_free(&set);

    assert(FOS_set_get_size(&set) == 0);
    assert(FOS_set_contains(&set, &values[0]) == false);

    /* Must be safe to free twice. */
    FOS_set_free(&set);
}

/* --------------------------------------------------------------------------
 * Main
 * -------------------------------------------------------------------------- */

int main(void)
{
    test_new();
    test_new_invalid();

    test_add_and_contains();
    test_duplicate_add();
    test_duplicate_by_value();

    test_remove();
    test_remove_and_readd();

    test_binary_elements();
    test_pointer_elements();

    test_collisions_and_probing();
    test_repeated_remove_and_add();

    test_iterate();
    test_iterate_empty();
    test_iterate_after_removal();

    test_invalid_arguments();

    test_large_set();

    test_free();

    printf("All FOS Set tests passed.\n");

    return 0;
}