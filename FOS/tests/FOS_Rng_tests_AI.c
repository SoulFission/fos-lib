#include "FOS_Rng.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

static bool array_contains(const int *arr, size_t n, int value)
{
    for (size_t i = 0; i < n; ++i)
    {
        if (arr[i] == value)
            return true;
    }

    return false;
}

/* --------------------------------------------------------------------------
 * Default / deterministic initialization
 * -------------------------------------------------------------------------- */

static void test_default_rng(void)
{
    FOS_Rng a = FOS_rng_default();
    FOS_Rng b = FOS_rng_default();

    for (size_t i = 0; i < 100; ++i)
        assert(FOS_rng_u32(&a) == FOS_rng_u32(&b));
}

static void test_seed_reproducibility(void)
{
    FOS_Rng a = FOS_rng_seed(123456789ULL, 987654321ULL);
    FOS_Rng b = FOS_rng_seed(123456789ULL, 987654321ULL);

    for (size_t i = 0; i < 1000; ++i)
        assert(FOS_rng_u32(&a) == FOS_rng_u32(&b));
}

static void test_different_seeds(void)
{
    FOS_Rng a = FOS_rng_seed(1, 1);
    FOS_Rng b = FOS_rng_seed(2, 1);

    bool different = false;

    for (size_t i = 0; i < 100; ++i)
    {
        if (FOS_rng_u32(&a) != FOS_rng_u32(&b))
        {
            different = true;
            break;
        }
    }

    assert(different);
}

static void test_different_streams(void)
{
    FOS_Rng a = FOS_rng_seed(12345, 1);
    FOS_Rng b = FOS_rng_seed(12345, 2);

    bool different = false;

    for (size_t i = 0; i < 100; ++i)
    {
        if (FOS_rng_u32(&a) != FOS_rng_u32(&b))
        {
            different = true;
            break;
        }
    }

    assert(different);
}

static void test_seed_auto(void)
{
    FOS_Rng a = FOS_rng_seed_auto();
    FOS_Rng b = FOS_rng_seed_auto();

    bool different = false;

    for (size_t i = 0; i < 100; ++i)
    {
        if (FOS_rng_u32(&a) != FOS_rng_u32(&b))
        {
            different = true;
            break;
        }
    }

    assert(different);
}

/* --------------------------------------------------------------------------
 * Basic output
 * -------------------------------------------------------------------------- */

static void test_u32(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    bool nonzero = false;

    for (size_t i = 0; i < 1000; ++i)
    {
        uint32_t value = FOS_rng_u32(&rng);

        if (value != 0)
            nonzero = true;
    }

    assert(nonzero);
}

static void test_u64(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    bool nonzero = false;

    for (size_t i = 0; i < 1000; ++i)
    {
        uint64_t value = FOS_rng_u64(&rng);

        if (value != 0)
            nonzero = true;
    }

    assert(nonzero);
}

/* --------------------------------------------------------------------------
 * Floating-point range
 * -------------------------------------------------------------------------- */

static void test_f64(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    bool nonzero = false;

    for (size_t i = 0; i < 10000; ++i)
    {
        double value = FOS_rng_f64(&rng);

        assert(value >= 0.0);
        assert(value < 1.0);

        if (value != 0.0)
            nonzero = true;
    }

    assert(nonzero);
}

/* --------------------------------------------------------------------------
 * uint32_t ranges
 * -------------------------------------------------------------------------- */

static void test_range_u32_equal(void)
{
    FOS_Rng rng = FOS_rng_seed(1, 1);

    for (uint32_t value = 0; value < 100; ++value)
    {
        for (size_t i = 0; i < 100; ++i)
            assert(FOS_rng_range_gen(&rng, value, value) == value);
    }
}

static void test_range_u32_two_values(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    bool seen_zero = false;
    bool seen_one = false;

    for (size_t i = 0; i < 10000; ++i)
    {
        uint32_t value = FOS_rng_range_gen(&rng, 0, 1);

        assert(value <= 1);

        if (value == 0)
            seen_zero = true;
        else
            seen_one = true;
    }

    assert(seen_zero);
    assert(seen_one);
}

static void test_range_u32_normal(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 100000; ++i)
    {
        uint32_t value = FOS_rng_range_gen(&rng, 10, 20);

        assert(value >= 10);
        assert(value <= 20);
    }
}

static void test_range_u32_reversed(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 100000; ++i)
    {
        uint32_t value = FOS_rng_range_gen(&rng, 20, 10);

        assert(value >= 10);
        assert(value <= 20);
    }
}

static void test_range_u32_boundary(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 100000; ++i)
    {
        uint32_t value;

        value = FOS_rng_range_gen(&rng, UINT32_MAX - 1, UINT32_MAX);
        assert(value == UINT32_MAX - 1 || value == UINT32_MAX);

        value = FOS_rng_range_gen(&rng, UINT32_MAX, UINT32_MAX);
        assert(value == UINT32_MAX);

        value = FOS_rng_range_gen(&rng, 0, UINT32_MAX);
        (void)value;

        /*
         * This is especially important after normalizing min/max inside
         * FOS_rng_range_gen().
         */
        value = FOS_rng_range_gen(&rng, UINT32_MAX, 0);
        (void)value;
    }
}

static void test_range_u32_full_range(void)
{
    FOS_Rng a = FOS_rng_seed(1234, 5678);
    FOS_Rng b = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 1000; ++i)
    {
        assert(FOS_rng_range_gen(&a, 0, UINT32_MAX) ==
               FOS_rng_u32(&b));
    }
}

/* --------------------------------------------------------------------------
 * uint64_t ranges
 * -------------------------------------------------------------------------- */

static void test_range_u64_equal(void)
{
    FOS_Rng rng = FOS_rng_seed(1, 1);

    uint64_t values[] = {
        0,
        1,
        UINT32_MAX,
        UINT64_MAX
    };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        for (size_t j = 0; j < 100; ++j)
            assert(FOS_rng_range_gen_u64(&rng, values[i], values[i]) ==
                   values[i]);
    }
}

static void test_range_u64_two_values(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    bool seen_zero = false;
    bool seen_one = false;

    for (size_t i = 0; i < 10000; ++i)
    {
        uint64_t value = FOS_rng_range_gen_u64(&rng, 0, 1);

        assert(value <= 1);

        if (value == 0)
            seen_zero = true;
        else
            seen_one = true;
    }

    assert(seen_zero);
    assert(seen_one);
}

static void test_range_u64_normal(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 100000; ++i)
    {
        uint64_t value = FOS_rng_range_gen_u64(&rng, 100, 1000);

        assert(value >= 100);
        assert(value <= 1000);
    }
}

static void test_range_u64_reversed(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 100000; ++i)
    {
        uint64_t value = FOS_rng_range_gen_u64(&rng, 1000, 100);

        assert(value >= 100);
        assert(value <= 1000);
    }
}

static void test_range_u64_boundary(void)
{
    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 100000; ++i)
    {
        uint64_t value;

        value = FOS_rng_range_gen_u64(
            &rng, UINT64_MAX - 1, UINT64_MAX);

        assert(value == UINT64_MAX - 1 || value == UINT64_MAX);

        value = FOS_rng_range_gen_u64(
            &rng, UINT64_MAX, UINT64_MAX);

        assert(value == UINT64_MAX);

        value = FOS_rng_range_gen_u64(
            &rng, 0, UINT64_MAX);

        (void)value;

        value = FOS_rng_range_gen_u64(
            &rng, UINT64_MAX, 0);

        (void)value;
    }
}

static void test_range_u64_full_range(void)
{
    FOS_Rng a = FOS_rng_seed(1234, 5678);
    FOS_Rng b = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 1000; ++i)
    {
        assert(FOS_rng_range_gen_u64(&a, 0, UINT64_MAX) ==
               FOS_rng_u64(&b));
    }
}

/* --------------------------------------------------------------------------
 * Global/thread-local range generator
 * -------------------------------------------------------------------------- */

static void test_range(void)
{
    for (size_t i = 0; i < 100000; ++i)
    {
        uint32_t value = FOS_rng_range(10, 20);

        assert(value >= 10);
        assert(value <= 20);
    }

    for (size_t i = 0; i < 1000; ++i)
        assert(FOS_rng_range(42, 42) == 42);
}

/* --------------------------------------------------------------------------
 * Integer shuffle
 * -------------------------------------------------------------------------- */

static void test_shuffle_int(void)
{
    int original[] = {
        0, 1, 2, 3, 4,
        5, 6, 7, 8, 9
    };

    int shuffled[10];

    memcpy(shuffled, original, sizeof(original));

    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    assert(FOS_rng_shuffle_int(shuffled, 10, &rng));

    /* Same elements must remain present. */
    for (size_t i = 0; i < 10; ++i)
        assert(array_contains(shuffled, 10, original[i]));

    /* No duplicates may have appeared. */
    for (size_t i = 0; i < 10; ++i)
    {
        for (size_t j = i + 1; j < 10; ++j)
            assert(shuffled[i] != shuffled[j]);
    }
}

static void test_shuffle_int_single(void)
{
    int value = 123;
    FOS_Rng rng = FOS_rng_seed(1, 1);

    assert(FOS_rng_shuffle_int(&value, 1, &rng));
    assert(value == 123);
}

static void test_shuffle_int_deterministic(void)
{
    int a[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int b[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    FOS_Rng rng_a = FOS_rng_seed(1234, 5678);
    FOS_Rng rng_b = FOS_rng_seed(1234, 5678);

    assert(FOS_rng_shuffle_int(a, 10, &rng_a));
    assert(FOS_rng_shuffle_int(b, 10, &rng_b));

    assert(memcmp(a, b, sizeof(a)) == 0);
}

/* --------------------------------------------------------------------------
 * Generic shuffle
 * -------------------------------------------------------------------------- */

typedef struct {
    int id;
    double value;
} Item;

static void test_shuffle_generic(void)
{
    Item original[] = {
        { 1, 1.5 },
        { 2, 2.5 },
        { 3, 3.5 },
        { 4, 4.5 },
        { 5, 5.5 }
    };

    Item shuffled[5];

    memcpy(shuffled, original, sizeof(original));

    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    assert(FOS_rng_shuffle_generic(
        shuffled,
        5,
        sizeof(Item),
        &rng
    ));

    /*
     * Every original structure must still occur exactly once.
     */
    for (size_t i = 0; i < 5; ++i)
    {
        bool found = false;

        for (size_t j = 0; j < 5; ++j)
        {
            if (memcmp(&original[i], &shuffled[j], sizeof(Item)) == 0)
            {
                assert(!found);
                found = true;
            }
        }

        assert(found);
    }
}

static void test_shuffle_generic_single(void)
{
    Item item = { 42, 3.14 };
    Item original = item;

    FOS_Rng rng = FOS_rng_seed(1, 1);

    assert(FOS_rng_shuffle_generic(
        &item,
        1,
        sizeof(item),
        &rng
    ));

    assert(memcmp(&item, &original, sizeof(item)) == 0);
}

/* --------------------------------------------------------------------------
 * Choose
 * -------------------------------------------------------------------------- */

static void test_choose(void)
{
    int values[] = { 10, 20, 30, 40, 50 };

    FOS_Rng rng = FOS_rng_seed(1234, 5678);

    for (size_t i = 0; i < 10000; ++i)
    {
        int *chosen = FOS_rng_choose(
            values,
            5,
            sizeof(int),
            &rng
        );

        assert(chosen != NULL);
        assert(chosen >= values);
        assert(chosen < values + 5);

        assert(*chosen == 10 ||
               *chosen == 20 ||
               *chosen == 30 ||
               *chosen == 40 ||
               *chosen == 50);
    }
}

static void test_choose_single(void)
{
    int value = 123;
    FOS_Rng rng = FOS_rng_seed(1, 1);

    int *chosen = FOS_rng_choose(
        &value,
        1,
        sizeof(value),
        &rng
    );

    assert(chosen == &value);
}

/* --------------------------------------------------------------------------
 * Invalid arguments
 * -------------------------------------------------------------------------- */

static void test_invalid_arguments(void)
{
    FOS_Rng rng = FOS_rng_seed(1, 1);

    int values[] = { 1, 2, 3 };

    assert(!FOS_rng_shuffle_int(NULL, 3, &rng));
    assert(!FOS_rng_shuffle_int(values, 0, &rng));
    assert(!FOS_rng_shuffle_int(values, 3, NULL));

    assert(!FOS_rng_shuffle_generic(NULL, 3, sizeof(int), &rng));
    assert(!FOS_rng_shuffle_generic(values, 0, sizeof(int), &rng));
    assert(!FOS_rng_shuffle_generic(values, 3, 0, &rng));
    assert(!FOS_rng_shuffle_generic(values, 3, sizeof(int), NULL));

    assert(FOS_rng_choose(NULL, 3, sizeof(int), &rng) == NULL);
    assert(FOS_rng_choose(values, 0, sizeof(int), &rng) == NULL);
    assert(FOS_rng_choose(values, 3, 0, &rng) == NULL);
    assert(FOS_rng_choose(values, 3, sizeof(int), NULL) == NULL);
}

/* --------------------------------------------------------------------------
 * Main
 * -------------------------------------------------------------------------- */

int main(void)
{
    test_default_rng();
    test_seed_reproducibility();
    test_different_seeds();
    test_different_streams();
    test_seed_auto();

    test_u32();
    test_u64();
    test_f64();

    test_range_u32_equal();
    test_range_u32_two_values();
    test_range_u32_normal();
    test_range_u32_reversed();
    test_range_u32_boundary();
    test_range_u32_full_range();

    test_range_u64_equal();
    test_range_u64_two_values();
    test_range_u64_normal();
    test_range_u64_reversed();
    test_range_u64_boundary();
    test_range_u64_full_range();

    test_range();

    test_shuffle_int();
    test_shuffle_int_single();
    test_shuffle_int_deterministic();

    test_shuffle_generic();
    test_shuffle_generic_single();

    test_choose();
    test_choose_single();

    test_invalid_arguments();

    printf("All FOS RNG tests passed.\n");

    return 0;
}