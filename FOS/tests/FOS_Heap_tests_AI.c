#include "FOS_Heap.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Comparators
 * ============================================================ */

static int int_cmp(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    return (x > y) - (x < y);
}

static int reverse_int_cmp(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    return (y > x) - (y < x);
}

typedef struct
{
    int priority;
    int id;
} Item;

static int item_cmp(const void *a, const void *b)
{
    const Item *x = a;
    const Item *y = b;

    return (x->priority > y->priority) - (x->priority < y->priority);
}

/* ============================================================
 * Helpers
 * ============================================================ */

/*
 * Verify the fundamental min-heap invariant:
 *
 *     parent <= left child
 *     parent <= right child
 *
 * The implementation stores elements in heap->vec.data using
 * the usual zero-based array representation.
 */
static void assert_heap_invariant(const FOS_Heap *heap)
{
    assert(heap != NULL);

    for (size_t i = 0; i < heap->vec.size; ++i)
    {
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;

        const char *base = heap->vec.data;

        const void *parent =
            base + i * heap->vec.elem_size;

        if (left < heap->vec.size)
        {
            const void *left_child =
                base + left * heap->vec.elem_size;

            assert(heap->cmp_func(parent, left_child) <= 0);
        }

        if (right < heap->vec.size)
        {
            const void *right_child =
                base + right * heap->vec.elem_size;

            assert(heap->cmp_func(parent, right_child) <= 0);
        }
    }
}

/*
 * Verify basic internal invariants as well as the heap invariant.
 */
static void assert_heap_valid(const FOS_Heap *heap)
{
    assert(heap != NULL);
    assert(heap->vec.elem_size > 0);
    assert(heap->cmp_func != NULL);
    assert(heap->swap_buf != NULL);

    assert_heap_invariant(heap);
}

static void assert_heap_empty(const FOS_Heap *heap)
{
    assert(heap != NULL);

    assert(heap->vec.size == 0);
    assert(FOS_heap_is_empty(heap));
    assert(FOS_heap_get_size(heap) == 0);

    assert_heap_valid(heap);
}

/* ============================================================
 * Initialization
 * ============================================================ */

static void test_init(void)
{
    printf("test_init...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    assert(heap.vec.data != NULL);
    assert(heap.vec.elem_size == sizeof(int));
    assert(heap.vec.size == 0);
    assert(heap.cmp_func == int_cmp);
    assert(heap.swap_buf != NULL);

    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

static void test_invalid_init(void)
{
    printf("test_invalid_init...\n");

    FOS_Heap heap;

    assert(!FOS_heap_init(NULL, int_cmp, sizeof(int)));
    assert(!FOS_heap_init(&heap, NULL, sizeof(int)));
    assert(!FOS_heap_init(&heap, int_cmp, 0));
}

static void test_failed_init_can_be_freed(void)
{
    printf("test_failed_init_can_be_freed...\n");

    FOS_Heap heap;

    /*
     * Failed initialization due to invalid arguments must not
     * make FOS_heap_free() crash.
     *
     * Since the current implementation does not initialize heap
     * when the arguments are invalid, initialize it to zero first.
     */
    memset(&heap, 0, sizeof(heap));

    assert(!FOS_heap_init(&heap, NULL, sizeof(int)));

    FOS_heap_free(&heap);

    assert(heap.vec.data == NULL);
    assert(heap.vec.size == 0);
    assert(heap.vec.elem_size == 0);
    assert(heap.swap_buf == NULL);
    assert(heap.cmp_func == NULL);
}

/* ============================================================
 * Push
 * ============================================================ */

static void test_push_single(void)
{
    printf("test_push_single...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int value = 42;

    assert(FOS_heap_push(&heap, &value));

    assert(FOS_heap_get_size(&heap) == 1);
    assert(!FOS_heap_is_empty(&heap));

    int out = 0;

    assert(FOS_heap_peek(&heap, &out));
    assert(out == 42);

    assert_heap_valid(&heap);

    FOS_heap_free(&heap);
}

static void test_push_order(void)
{
    printf("test_push_order...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int values[] = {5, 3, 8, 1, 7, 2, 4};

    for (size_t i = 0; i < 7; ++i)
    {
        assert(FOS_heap_push(&heap, &values[i]));
        assert_heap_valid(&heap);
    }

    assert(FOS_heap_get_size(&heap) == 7);

    /*
     * The minimum must always be at the root.
     */
    int root = 0;

    assert(FOS_heap_peek(&heap, &root));
    assert(root == 1);

    FOS_heap_free(&heap);
}

/* ============================================================
 * Peek
 * ============================================================ */

static void test_peek(void)
{
    printf("test_peek...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int values[] = {10, 4, 7, 2, 9};

    for (size_t i = 0; i < 5; ++i)
        assert(FOS_heap_push(&heap, &values[i]));

    int out = -1;

    assert(FOS_heap_peek(&heap, &out));
    assert(out == 2);

    /*
     * Peek must not remove anything.
     */
    assert(FOS_heap_get_size(&heap) == 5);

    assert(FOS_heap_peek(&heap, &out));
    assert(out == 2);

    assert_heap_valid(&heap);

    FOS_heap_free(&heap);
}

static void test_peek_empty(void)
{
    printf("test_peek_empty...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int out;

    assert(!FOS_heap_peek(&heap, &out));
    assert(!FOS_heap_peek(&heap, NULL));

    FOS_heap_free(&heap);
}

/* ============================================================
 * Pop
 * ============================================================ */

static void test_pop_sorted(void)
{
    printf("test_pop_sorted...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int values[] = {5, 3, 8, 1, 7, 2, 4};

    for (size_t i = 0; i < 7; ++i)
    {
        assert(FOS_heap_push(&heap, &values[i]));
        assert_heap_valid(&heap);
    }

    int expected[] = {1, 2, 3, 4, 5, 7, 8};

    for (size_t i = 0; i < 7; ++i)
    {
        int out = 0;

        assert(FOS_heap_pop(&heap, &out));
        assert(out == expected[i]);

        assert_heap_valid(&heap);
        assert(FOS_heap_get_size(&heap) == 7 - i - 1);
    }

    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

static void test_pop_empty(void)
{
    printf("test_pop_empty...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int out;

    assert(!FOS_heap_pop(&heap, &out));
    assert(!FOS_heap_pop(&heap, NULL));

    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

static void test_pop_single(void)
{
    printf("test_pop_single...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int value = 123;
    int out = 0;

    assert(FOS_heap_push(&heap, &value));
    assert(FOS_heap_pop(&heap, &out));

    assert(out == 123);
    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

/* ============================================================
 * Duplicates
 * ============================================================ */

static void test_duplicates(void)
{
    printf("test_duplicates...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int values[] =
    {
        5, 2, 5, 1, 2, 1, 5, 3, 3
    };

    for (size_t i = 0; i < 9; ++i)
    {
        assert(FOS_heap_push(&heap, &values[i]));
        assert_heap_valid(&heap);
    }

    int expected[] =
    {
        1, 1, 2, 2, 3, 3, 5, 5, 5
    };

    for (size_t i = 0; i < 9; ++i)
    {
        int out;

        assert(FOS_heap_pop(&heap, &out));
        assert(out == expected[i]);

        assert_heap_valid(&heap);
    }

    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

/* ============================================================
 * Negative and extreme values
 * ============================================================ */

static void test_negative_values(void)
{
    printf("test_negative_values...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int values[] =
    {
        -10,
        5,
        -100,
        20,
        0,
        -1,
        50
    };

    for (size_t i = 0; i < 7; ++i)
    {
        assert(FOS_heap_push(&heap, &values[i]));
        assert_heap_valid(&heap);
    }

    int expected[] =
    {
        -100,
        -10,
        -1,
        0,
        5,
        20,
        50
    };

    for (size_t i = 0; i < 7; ++i)
    {
        int out;

        assert(FOS_heap_pop(&heap, &out));
        assert(out == expected[i]);
    }

    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

static void test_extreme_values(void)
{
    printf("test_extreme_values...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int values[] =
    {
        INT_MAX,
        INT_MIN,
        0,
        -1,
        1
    };

    for (size_t i = 0; i < 5; ++i)
        assert(FOS_heap_push(&heap, &values[i]));

    int expected[] =
    {
        INT_MIN,
        -1,
        0,
        1,
        INT_MAX
    };

    for (size_t i = 0; i < 5; ++i)
    {
        int out;

        assert(FOS_heap_pop(&heap, &out));
        assert(out == expected[i]);
        assert_heap_valid(&heap);
    }

    FOS_heap_free(&heap);
}

/* ============================================================
 * Custom comparator
 * ============================================================ */

static void test_reverse_comparator(void)
{
    printf("test_reverse_comparator...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, reverse_int_cmp, sizeof(int)));

    int values[] = {5, 1, 8, 3, 7, 2, 4};

    for (size_t i = 0; i < 7; ++i)
    {
        assert(FOS_heap_push(&heap, &values[i]));
        assert_heap_valid(&heap);
    }

    /*
     * With reverse_int_cmp(), the heap behaves as a max-heap.
     */
    int expected[] = {8, 7, 5, 4, 3, 2, 1};

    for (size_t i = 0; i < 7; ++i)
    {
        int out;

        assert(FOS_heap_pop(&heap, &out));
        assert(out == expected[i]);

        assert_heap_valid(&heap);
    }

    FOS_heap_free(&heap);
}

/* ============================================================
 * Struct elements
 * ============================================================ */

static void test_struct_elements(void)
{
    printf("test_struct_elements...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, item_cmp, sizeof(Item)));

    Item values[] =
    {
        {50, 0},
        {10, 1},
        {30, 2},
        {5,  3},
        {20, 4}
    };

    for (size_t i = 0; i < 5; ++i)
    {
        assert(FOS_heap_push(&heap, &values[i]));
        assert_heap_valid(&heap);
    }

    int expected_priorities[] = {5, 10, 20, 30, 50};

    for (size_t i = 0; i < 5; ++i)
    {
        Item out;

        assert(FOS_heap_pop(&heap, &out));
        assert(out.priority == expected_priorities[i]);
    }

    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

/* ============================================================
 * Invalid arguments
 * ============================================================ */

static void test_invalid_arguments(void)
{
    printf("test_invalid_arguments...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int value = 42;
    int out;

    assert(!FOS_heap_push(NULL, &value));
    assert(!FOS_heap_push(&heap, NULL));

    assert(!FOS_heap_pop(NULL, &out));
    assert(!FOS_heap_pop(&heap, NULL));

    assert(!FOS_heap_peek(NULL, &out));
    assert(!FOS_heap_peek(&heap, NULL));

    

    assert(!FOS_heap_is_empty(NULL));
    assert(FOS_heap_get_size(NULL) == SIZE_MAX);

    FOS_heap_free(&heap);
}

/* ============================================================
 * Large sequence
 * ============================================================ */

static void test_many_elements(void)
{
    printf("test_many_elements...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    /*
     * Insert a deliberately non-sorted sequence.
     */
    for (int i = 999; i >= 0; --i)
    {
        assert(FOS_heap_push(&heap, &i));
        assert_heap_invariant(&heap);
    }

    assert(FOS_heap_get_size(&heap) == 1000);

    for (int expected = 0; expected < 1000; ++expected)
    {
        int out;

        assert(FOS_heap_pop(&heap, &out));
        assert(out == expected);

        assert_heap_invariant(&heap);
    }

    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

/* ============================================================
 * Alternating push/pop
 * ============================================================ */

static void test_alternating_operations(void)
{
    printf("test_alternating_operations...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    int a = 50;
    int b = 20;
    int c = 70;
    int d = 10;
    int e = 30;

    assert(FOS_heap_push(&heap, &a));
    assert_heap_valid(&heap);

    assert(FOS_heap_push(&heap, &b));
    assert_heap_valid(&heap);

    {
        int out;
        assert(FOS_heap_pop(&heap, &out));
        assert(out == 20);
        assert_heap_valid(&heap);
    }

    assert(FOS_heap_push(&heap, &c));
    assert_heap_valid(&heap);

    assert(FOS_heap_push(&heap, &d));
    assert_heap_valid(&heap);

    {
        int out;
        assert(FOS_heap_pop(&heap, &out));
        assert(out == 10);
        assert_heap_valid(&heap);
    }

    assert(FOS_heap_push(&heap, &e));
    assert_heap_valid(&heap);

    {
        int out;

        assert(FOS_heap_pop(&heap, &out));
        assert(out == 30);

        assert(FOS_heap_pop(&heap, &out));
        assert(out == 50);

        assert(FOS_heap_pop(&heap, &out));
        assert(out == 70);
    }

    assert_heap_empty(&heap);

    FOS_heap_free(&heap);
}

/* ============================================================
 * Free
 * ============================================================ */

static void test_free(void)
{
    printf("test_free...\n");

    FOS_Heap heap;

    assert(FOS_heap_init(&heap, int_cmp, sizeof(int)));

    for (int i = 0; i < 100; ++i)
        assert(FOS_heap_push(&heap, &i));

    FOS_heap_free(&heap);

    assert(heap.vec.data == NULL);
    assert(heap.vec.size == 0);
    assert(heap.vec.elem_size == 0);
    assert(heap.swap_buf == NULL);
    assert(heap.cmp_func == NULL);

    /*
     * Double free should be safe because FOS_heap_free()
     * zeroes the object.
     */
    FOS_heap_free(&heap);

    assert(heap.vec.data == NULL);
    assert(heap.vec.size == 0);
    assert(heap.vec.elem_size == 0);
    assert(heap.swap_buf == NULL);
    assert(heap.cmp_func == NULL);
}

/* ============================================================
 * Main
 * ============================================================ */

int main(void)
{
    printf("=== FOS Heap Tests ===\n\n");

    test_init();
    test_invalid_init();
    test_failed_init_can_be_freed();

    test_push_single();
    test_push_order();

    test_peek();
    test_peek_empty();

    test_pop_sorted();
    test_pop_empty();
    test_pop_single();

    test_duplicates();
    test_negative_values();
    test_extreme_values();

    test_reverse_comparator();
    test_struct_elements();

    test_invalid_arguments();

    test_many_elements();
    test_alternating_operations();

    test_free();

    printf("\n=== ALL FOS HEAP TESTS PASSED ===\n");

    return 0;
}
