/*
 * FOS_Vec test harness
 *
 * Intended to catch:
 *   - incorrect size/capacity handling
 *   - reserve/growth failures
 *   - insert/erase boundary errors
 *   - aliasing across reallocations
 *   - copy/resize mistakes
 *   - shrink-to-fit mistakes
 *   - sorting errors
 *   - traversal macro errors
 *
 * Compile with sanitizers when possible:
 *
 *   GCC/Clang:
 *     -fsanitize=address,undefined -fno-omit-frame-pointer
 *
 * Example:
 *   cc -std=c11 -Wall -Wextra -Wpedantic \
 *      -fsanitize=address,undefined -fno-omit-frame-pointer \
 *      test_vec.c FOS_Vec.c FOS_Memory.c -o test_vec
 */

#include "FOS_Vec.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ------------------------------------------------------------------------- */
/* Test helpers                                                              */
/* ------------------------------------------------------------------------- */

#define CHECK(expr)                                                         \
    do                                                                      \
    {                                                                       \
        if (!(expr))                                                        \
        {                                                                   \
            fprintf(stderr,                                                   \
                    "FAIL: %s\n    at %s:%d\n",                              \
                    #expr, __FILE__, __LINE__);                              \
            exit(EXIT_FAILURE);                                              \
        }                                                                   \
    } while (0)

#define CHECK_EQ(actual, expected)                                          \
    do                                                                      \
    {                                                                       \
        if ((actual) != (expected))                                         \
        {                                                                   \
            fprintf(stderr,                                                   \
                    "FAIL: %s == %s\n    actual: %zu\n    expected: %zu\n"   \
                    "    at %s:%d\n",                                       \
                    #actual, #expected,                                     \
                    (size_t)(actual), (size_t)(expected),                   \
                    __FILE__, __LINE__);                                    \
            exit(EXIT_FAILURE);                                              \
        }                                                                   \
    } while (0)


static int int_compare(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    return (x > y) - (x < y);
}


static bool vector_is_valid(const FOS_Vec *vec)
{
    if (vec == NULL)
        return false;

    if (vec->data == NULL)
        return false;

    if (vec->elem_size == 0)
        return false;

    if (vec->capacity < FOS_VEC_INIT_CAP)
        return false;

    if (vec->size > vec->capacity)
        return false;

    if (vec->capacity > SIZE_MAX / vec->elem_size)
        return false;

    return true;
}


/* ------------------------------------------------------------------------- */
/* Construction                                                              */
/* ------------------------------------------------------------------------- */

static void test_new(void)
{
    puts("  new/free");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    CHECK(vector_is_valid(&vec));
    CHECK_EQ(vec.size, 0);
    CHECK_EQ(vec.capacity, FOS_VEC_INIT_CAP);
    CHECK_EQ(vec.elem_size, sizeof(int));

    FOS_vec_free(&vec);

    CHECK(vec.data == NULL);
    CHECK_EQ(vec.size, 0);
    CHECK_EQ(vec.capacity, 0);
    CHECK_EQ(vec.elem_size, 0);

    CHECK(FOS_vec_new(0).data == NULL);
}


/* ------------------------------------------------------------------------- */
/* Push/pop                                                                  */
/* ------------------------------------------------------------------------- */

static void test_push_pop(void)
{
    puts("  push/pop");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 1000; ++i)
    {
        CHECK(FOS_vec_push(&vec, &i));
        CHECK_EQ(vec.size, (size_t)i + 1);
        CHECK(vector_is_valid(&vec));
    }

    for (int i = 999; i >= 0; --i)
    {
        int value = -1;

        CHECK(FOS_vec_pop(&vec, &value));
        CHECK_EQ(value, (size_t)i);
        CHECK_EQ(vec.size, (size_t)i);
        CHECK(vector_is_valid(&vec));
    }

    CHECK(!FOS_vec_pop(&vec, &(int){0}));

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Push aliasing                                                             */
/* ------------------------------------------------------------------------- */

static void test_push_aliasing(void)
{
    puts("  push aliasing");

    /*
     * Force the vector to be full so push() must realloc().
     *
     * The source element is inside the vector. If realloc moves the
     * allocation, the original pointer becomes invalid.
     */
    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < (int)vec.capacity; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    int *source = FOS_vec_at(&vec, 3);

    CHECK(source != NULL);
    CHECK(*source == 3);

    CHECK(FOS_vec_push(&vec, source));

    CHECK_EQ(vec.size, vec.capacity / 2 + 1);
    CHECK(*(int *)FOS_vec_at(&vec, 3) == 3);
    CHECK(*(int *)FOS_vec_at(&vec, vec.size - 1) == 3);

    /*
     * Another aliasing case with the last element.
     */
    source = FOS_vec_at(&vec, vec.size - 1);
    CHECK(source != NULL);

    CHECK(FOS_vec_push(&vec, source));
    CHECK(*(int *)FOS_vec_at(&vec, vec.size - 1) == 3);

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Reserve                                                                  */
/* ------------------------------------------------------------------------- */

static void test_reserve(void)
{
    puts("  reserve");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    size_t old_capacity = vec.capacity;

    CHECK(FOS_vec_reserve(&vec, old_capacity));
    CHECK_EQ(vec.capacity, old_capacity);

    CHECK(FOS_vec_reserve(&vec, old_capacity + 100));
    CHECK_EQ(vec.capacity, old_capacity + 100);

    /*
     * Reserving less than the current capacity must do nothing.
     */
    CHECK(FOS_vec_reserve(&vec, 1));
    CHECK_EQ(vec.capacity, old_capacity + 100);

    /*
     * Overflow must be rejected.
     */
    CHECK(!FOS_vec_reserve(&vec, SIZE_MAX));

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* At/data                                                                   */
/* ------------------------------------------------------------------------- */

static void test_at_data(void)
{
    puts("  at/data");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 20; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    for (size_t i = 0; i < vec.size; ++i)
    {
        int *value = FOS_vec_at(&vec, i);

        CHECK(value != NULL);
        CHECK_EQ(*value, i);
    }

    CHECK(FOS_vec_at(&vec, vec.size) == NULL);
    CHECK(FOS_vec_at(&vec, SIZE_MAX) == NULL);

    int *data = FOS_vec_data(&vec);

    CHECK(data != NULL);

    for (size_t i = 0; i < vec.size; ++i)
        CHECK_EQ(data[i], i);

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Clear                                                                     */
/* ------------------------------------------------------------------------- */

static void test_clear(void)
{
    puts("  clear");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 100; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    size_t capacity = vec.capacity;

    FOS_vec_clear(&vec);

    CHECK_EQ(vec.size, 0);
    CHECK_EQ(vec.capacity, capacity);
    CHECK(vec.data != NULL);
    CHECK(vector_is_valid(&vec));

    /*
     * A cleared vector must remain usable.
     */
    int value = 42;

    CHECK(FOS_vec_push(&vec, &value));
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 0), 42);

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Resize                                                                    */
/* ------------------------------------------------------------------------- */

static void test_resize(void)
{
    puts("  resize");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    CHECK(FOS_vec_resize(&vec, 100));
    CHECK_EQ(vec.size, 100);
    CHECK(vector_is_valid(&vec));

    /*
     * Newly exposed storage is byte-zeroed. For int this should produce 0.
     */
    for (size_t i = 0; i < vec.size; ++i)
        CHECK(*(int *)FOS_vec_at(&vec, i) == 0);

    /*
     * Shrinking must preserve the prefix.
     */
    for (size_t i = 0; i < 50; ++i)
        *(int *)FOS_vec_at(&vec, i) = (int)i;

    CHECK(FOS_vec_resize(&vec, 50));
    CHECK_EQ(vec.size, 50);

    for (size_t i = 0; i < vec.size; ++i)
        CHECK(*(int *)FOS_vec_at(&vec, i) == (int)i);

    /*
     * Growing again must preserve the old prefix.
     */
    CHECK(FOS_vec_resize(&vec, 200));

    for (size_t i = 0; i < 50; ++i)
        CHECK(*(int *)FOS_vec_at(&vec, i) == (int)i);

    for (size_t i = 50; i < 200; ++i)
        CHECK(*(int *)FOS_vec_at(&vec, i) == 0);

    CHECK(vector_is_valid(&vec));

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Insert                                                                    */
/* ------------------------------------------------------------------------- */

static void test_insert(void)
{
    puts("  insert");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 10; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    /*
     * Insert at beginning.
     */
    int value = 100;

    CHECK(FOS_vec_insert(&vec, 0, &value));
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 0), 100);
    CHECK_EQ(vec.size, 11);

    /*
     * Insert in the middle.
     */
    value = 200;

    CHECK(FOS_vec_insert(&vec, 5, &value));
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 5), 200);
    CHECK_EQ(vec.size, 12);

    /*
     * Insert at end is delegated to push().
     */
    value = 300;

    CHECK(FOS_vec_insert(&vec, vec.size, &value));
    CHECK_EQ(*(int *)FOS_vec_at(&vec, vec.size - 1), 300);
    CHECK_EQ(vec.size, 13);

    /*
     * Invalid index.
     */
    CHECK(!FOS_vec_insert(&vec, vec.size + 1, &value));

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Insert aliasing                                                           */
/* ------------------------------------------------------------------------- */

static void test_insert_aliasing(void)
{
    puts("  insert aliasing");

    /*
     * Case 1:
     *
     * Insert an element before its current position.
     * No realloc is necessary here.
     */
    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 8; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    int *source = FOS_vec_at(&vec, 5);

    CHECK(source != NULL);
    CHECK(FOS_vec_insert(&vec, 2, source));

    /*
     * Original element 5 moves from index 5 to index 6.
     * Inserted copy is at index 2.
     */
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 2), 5);
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 6), 5);

    FOS_vec_free(&vec);

    /*
     * Case 2:
     *
     * Force realloc and insert an aliased element.
     */
    vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < (int)vec.capacity; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    source = FOS_vec_at(&vec, 10);

    CHECK(source != NULL);
    CHECK(FOS_vec_insert(&vec, 3, source));

    CHECK_EQ(*(int *)FOS_vec_at(&vec, 3), 10);
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 11), 10);

    /*
     * Alias from before the insertion point.
     *
     * This source should not have its index adjusted.
     */
    source = FOS_vec_at(&vec, 1);

    CHECK(FOS_vec_insert(&vec, 8, source));

    CHECK_EQ(*(int *)FOS_vec_at(&vec, 8), 1);
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 1), 1);

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Erase                                                                    */
/* ------------------------------------------------------------------------- */

static void test_erase(void)
{
    puts("  erase");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 20; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    /*
     * erase_at
     */
    CHECK(FOS_vec_erase_at(&vec, 0));
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 0), 1);

    CHECK(FOS_vec_erase_at(&vec, vec.size - 1));
    CHECK_EQ(*(int *)FOS_vec_at(&vec, vec.size - 1), 18);

    /*
     * erase_range
     */
    CHECK(FOS_vec_erase_range(&vec, 3, 7));
    CHECK_EQ(vec.size, 13);

    /*
     * Invalid ranges.
     */
    CHECK(!FOS_vec_erase_range(&vec, 7, 3));
    CHECK(!FOS_vec_erase_range(&vec, 0, vec.size));
    CHECK(!FOS_vec_erase_at(&vec, vec.size));

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Unordered erase                                                           */
/* ------------------------------------------------------------------------- */

static void test_erase_unordered(void)
{
    puts("  unordered erase");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 10; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    CHECK(FOS_vec_erase_unordered(&vec, 3));

    CHECK_EQ(vec.size, 9);

    /*
     * Element 9 should have replaced element 3.
     */
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 3), 9);

    CHECK(FOS_vec_erase_unordered(&vec, vec.size - 1));
    CHECK_EQ(vec.size, 8);

    CHECK(!FOS_vec_erase_unordered(&vec, vec.size));

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Copy                                                                      */
/* ------------------------------------------------------------------------- */

static void test_copy(void)
{
    puts("  copy");

    FOS_Vec src = FOS_vec_new(sizeof(int));
    FOS_Vec dst = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 1000; ++i)
        CHECK(FOS_vec_push(&src, &i));

    CHECK(FOS_vec_copy(&dst, &src));

    CHECK_EQ(dst.size, src.size);
    CHECK(dst.elem_size == src.elem_size);
    CHECK(vector_is_valid(&dst));

    for (size_t i = 0; i < src.size; ++i)
        CHECK(*(int *)FOS_vec_at(&dst, i) == *(int *)FOS_vec_at(&src, i));

    /*
     * Self-copy.
     */
    CHECK(FOS_vec_copy(&src, &src));

    CHECK_EQ(src.size, 1000);

    /*
     * Different element sizes must be rejected.
     */
    FOS_Vec other = FOS_vec_new(sizeof(double));

    CHECK(!FOS_vec_copy(&dst, &other));

    FOS_vec_free(&other);
    FOS_vec_free(&dst);
    FOS_vec_free(&src);
}


/* ------------------------------------------------------------------------- */
/* Shrink-to-fit                                                             */
/* ------------------------------------------------------------------------- */

static void test_shrink_to_fit(void)
{
    puts("  shrink_to_fit");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 1000; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    CHECK(FOS_vec_resize(&vec, 100));

    CHECK(FOS_vec_shrink_to_fit(&vec));

    CHECK_EQ(vec.capacity, vec.size);
    CHECK_EQ(vec.size, 100);
    CHECK(vector_is_valid(&vec));

    for (size_t i = 0; i < vec.size; ++i)
        CHECK(*(int *)FOS_vec_at(&vec, i) == (int)i);

    /*
     * Empty vector must remain usable after shrink_to_fit().
     */
    FOS_vec_clear(&vec);

    CHECK(FOS_vec_shrink_to_fit(&vec));

    CHECK_EQ(vec.size, 0);
    CHECK(vec.data != NULL);
    CHECK_EQ(vec.capacity, FOS_VEC_INIT_CAP);
    CHECK(vector_is_valid(&vec));

    int value = 123;

    CHECK(FOS_vec_push(&vec, &value));
    CHECK_EQ(*(int *)FOS_vec_at(&vec, 0), 123);

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Sorting                                                                    */
/* ------------------------------------------------------------------------- */

static void test_sort(void)
{
    puts("  sort");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    const int values[] =
    {
        9, 2, 7, 1, 8, 3, 6, 4, 0, 5,
        5, 5, 1, 9, 0
    };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
        CHECK(FOS_vec_push(&vec, &values[i]));

    CHECK(FOS_vec_sort(&vec, int_compare));

    for (size_t i = 1; i < vec.size; ++i)
    {
        int a = *(int *)FOS_vec_at(&vec, i - 1);
        int b = *(int *)FOS_vec_at(&vec, i);

        CHECK(a <= b);
    }

    CHECK(!FOS_vec_sort(&vec, NULL));

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Traversal macros                                                          */
/* ------------------------------------------------------------------------- */

static void test_iteration(void)
{
    puts("  iteration macros");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 1; i <= 10; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    int expected = 1;

    FOS_VEC_FOR_EACH(int, &vec, it)
    {
        CHECK(*it == expected);
        ++expected;
    }

    CHECK(expected == 11);

    expected = 10;

    FOS_VEC_FOR_EACH_REV(int, &vec, it)
    {
        CHECK(*it == expected);
        --expected;
    }

    CHECK(expected == 0);

    /*
     * Empty vector: neither traversal should execute.
     */
    FOS_vec_clear(&vec);

    size_t count = 0;

    FOS_VEC_FOR_EACH(int, &vec, it)
    {
        (void)it;
        ++count;
    }

    CHECK_EQ(count, 0);

    count = 0;

    FOS_VEC_FOR_EACH_REV(int, &vec, it)
    {
        (void)it;
        ++count;
    }

    CHECK_EQ(count, 0);

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Raw traversal                                                             */
/* ------------------------------------------------------------------------- */

static void test_raw_iteration(void)
{
    puts("  raw iteration");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    for (int i = 0; i < 100; ++i)
        CHECK(FOS_vec_push(&vec, &i));

    size_t count = 0;
    int expected = 0;

    FOS_VEC_FOR_EACH_RAW(&vec, raw)
    {
        int value;

        memcpy(&value, raw, sizeof(value));

        CHECK(value == expected);

        ++expected;
        ++count;
    }

    CHECK_EQ(count, 100);

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Typed wrapper                                                              */
/* ------------------------------------------------------------------------- */

FOS_VEC_DEFINE(int, Int)


static void test_typed_wrapper(void)
{
    puts("  typed wrapper");

    FOS_Vec_Int vec = FOS_vec_Int_new();

    for (int i = 0; i < 100; ++i)
        CHECK(FOS_vec_Int_push(&vec, i));

    CHECK_EQ(vec.size, 100);

    CHECK(FOS_vec_Int_at(&vec, 10) != NULL);
    CHECK_EQ(*FOS_vec_Int_at(&vec, 10), 10);

    CHECK(FOS_vec_Int_sort(&vec, int_compare));

    for (size_t i = 1; i < vec.size; ++i)
        CHECK(vec.data != NULL);

    FOS_vec_Int_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Repeated mutation stress test                                             */
/* ------------------------------------------------------------------------- */

static void test_mutation_stress(void)
{
    puts("  mutation stress");

    FOS_Vec vec = FOS_vec_new(sizeof(int));

    /*
     * Repeatedly perform different operations. This is not a formal
     * randomized property test, but it exercises many state transitions.
     */
    for (int round = 0; round < 100; ++round)
    {
        FOS_vec_clear(&vec);

        for (int i = 0; i < 500; ++i)
        {
            int value = i;

            CHECK(FOS_vec_push(&vec, &value));
        }

        for (int i = 0; i < 100; ++i)
        {
            size_t index = (size_t)((i * 37) % (int)vec.size);

            int value = -i;

            CHECK(FOS_vec_insert(&vec, index, &value));
        }

        for (int i = 0; i < 100; ++i)
        {
            size_t index = (size_t)((i * 17) % (int)vec.size);

            CHECK(FOS_vec_erase_at(&vec, index));
        }

        CHECK(vector_is_valid(&vec));

        CHECK(FOS_vec_sort(&vec, int_compare));

        for (size_t i = 1; i < vec.size; ++i)
        {
            int a = *(int *)FOS_vec_at(&vec, i - 1);
            int b = *(int *)FOS_vec_at(&vec, i);

            CHECK(a <= b);
        }
    }

    FOS_vec_free(&vec);
}


/* ------------------------------------------------------------------------- */
/* Main                                                                      */
/* ------------------------------------------------------------------------- */

int main(void)
{
    puts("FOS_Vec test suite");
    puts("------------------");

    test_new();
    test_push_pop();
    test_push_aliasing();
    test_reserve();
    test_at_data();
    test_clear();
    test_resize();
    test_insert();
    test_insert_aliasing();
    test_erase();
    test_erase_unordered();
    test_copy();
    test_shrink_to_fit();
    test_sort();
    test_iteration();
    test_raw_iteration();
    test_typed_wrapper();
    test_mutation_stress();

    puts("------------------");
    puts("ALL TESTS PASSED");

    return EXIT_SUCCESS;
}