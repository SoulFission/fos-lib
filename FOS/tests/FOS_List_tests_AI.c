#include "FOS_List.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* ============================================================
 * Helpers
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
    int key;
    int id;
} Item;

static int item_cmp(const void *a, const void *b)
{
    const Item *x = a;
    const Item *y = b;

    return (x->key > y->key) - (x->key < y->key);
}

static void assert_int_list(
    const FOS_List *list,
    const int *expected,
    size_t count
)
{
    assert(list != NULL);
    assert(FOS_list_size(list) == count);
    assert(FOS_list_is_empty(list) == (count == 0));

    for (size_t i = 0; i < count; ++i)
    {
        const int *value = FOS_list_at(list, i);

        assert(value != NULL);
        assert(*value == expected[i]);
    }

    assert(FOS_list_at(list, count) == NULL);

    if (count == 0)
    {
        assert(FOS_list_front(list) == NULL);
        assert(FOS_list_back(list) == NULL);
        assert(list->head == NULL);
        assert(list->tail == NULL);
    }
    else
    {
        assert(FOS_list_front(list) != NULL);
        assert(FOS_list_back(list) != NULL);

        assert(*(const int *)FOS_list_front(list) == expected[0]);
        assert(*(const int *)FOS_list_back(list) == expected[count - 1]);

        assert(list->head != NULL);
        assert(list->tail != NULL);
        assert(list->tail->next == NULL);
    }
}

/*
 * Verify structural invariants independently of the public API.
 *
 * This catches bugs where size/head/tail become inconsistent even
 * when simple element tests happen to pass.
 */
static void assert_list_invariants(const FOS_List *list)
{
    assert(list != NULL);

    if (list->size == 0)
    {
        assert(list->head == NULL);
        assert(list->tail == NULL);
        return;
    }

    assert(list->head != NULL);
    assert(list->tail != NULL);
    assert(list->tail->next == NULL);

    size_t count = 0;
    const FOS_ListNode *current = list->head;
    const FOS_ListNode *last = NULL;

    while (current != NULL)
    {
        assert(current->data != NULL);

        last = current;
        current = current->next;
        ++count;

        /*
         * If there is an accidental cycle, don't allow the test
         * to loop forever.
         */
        assert(count <= list->size);
    }

    assert(count == list->size);
    assert(last == list->tail);
}

static void print_int_list(const FOS_List *list)
{
    printf("[ ");

    for (size_t i = 0; i < list->size; ++i)
        printf("%d ", *(const int *)FOS_list_at(list, i));

    printf("]\n");
}

/* ============================================================
 * Initialization
 * ============================================================ */

static void test_init(void)
{
    printf("test_init...\n");

    FOS_List list;

    assert(FOS_list_init(&list, sizeof(int)));

    assert(list.head == NULL);
    assert(list.tail == NULL);
    assert(list.elem_size == sizeof(int));
    assert(list.size == 0);

    assert(FOS_list_is_empty(&list));
    assert(FOS_list_size(&list) == 0);

    assert_list_invariants(&list);

    FOS_list_free(&list);
}

static void test_invalid_init(void)
{
    printf("test_invalid_init...\n");

    FOS_List list;

    assert(!FOS_list_init(NULL, sizeof(int)));
    assert(!FOS_list_init(&list, 0));
}

/* ============================================================
 * Push front / back
 * ============================================================ */

static void test_push_front(void)
{
    printf("test_push_front...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3, 4, 5};

    for (size_t i = 0; i < 5; ++i)
    {
        assert(FOS_list_push_front(&list, &values[i]));
        assert_list_invariants(&list);
    }

    int expected[] = {5, 4, 3, 2, 1};

    assert_int_list(&list, expected, 5);

    FOS_list_free(&list);
}

static void test_push_back(void)
{
    printf("test_push_back...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3, 4, 5};

    for (size_t i = 0; i < 5; ++i)
    {
        assert(FOS_list_push_back(&list, &values[i]));
        assert_list_invariants(&list);
    }

    assert_int_list(&list, values, 5);

    FOS_list_free(&list);
}

static void test_mixed_push(void)
{
    printf("test_mixed_push...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int a = 2;
    int b = 1;
    int c = 3;
    int d = 0;

    assert(FOS_list_push_back(&list, &a));
    assert(FOS_list_push_front(&list, &b));
    assert(FOS_list_push_back(&list, &c));
    assert(FOS_list_push_front(&list, &d));

    int expected[] = {0, 1, 2, 3};

    assert_int_list(&list, expected, 4);
    assert_list_invariants(&list);

    FOS_list_free(&list);
}

/* ============================================================
 * Pop front / back
 * ============================================================ */

static void test_pop_front(void)
{
    printf("test_pop_front...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3, 4};

    for (size_t i = 0; i < 4; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    for (int expected = 1; expected <= 4; ++expected)
    {
        int out = -1;

        assert(FOS_list_pop_front(&list, &out));
        assert(out == expected);

        assert_list_invariants(&list);
    }

    assert(FOS_list_is_empty(&list));
    assert(!FOS_list_pop_front(&list, NULL));

    FOS_list_free(&list);
}

static void test_pop_front_without_output(void)
{
    printf("test_pop_front_without_output...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int value = 42;

    assert(FOS_list_push_back(&list, &value));
    assert(FOS_list_pop_front(&list, NULL));

    assert(FOS_list_is_empty(&list));
    assert_list_invariants(&list);

    FOS_list_free(&list);
}

static void test_pop_back(void)
{
    printf("test_pop_back...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3, 4};

    for (size_t i = 0; i < 4; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    for (int expected = 4; expected >= 1; --expected)
    {
        int out = -1;

        assert(FOS_list_pop_back(&list, &out));
        assert(out == expected);

        assert_list_invariants(&list);
    }

    assert(FOS_list_is_empty(&list));
    assert(!FOS_list_pop_back(&list, NULL));

    FOS_list_free(&list);
}

static void test_single_element_pop(void)
{
    printf("test_single_element_pop...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int value = 123;
    int out = 0;

    assert(FOS_list_push_back(&list, &value));

    assert(FOS_list_pop_back(&list, &out));
    assert(out == 123);

    assert(list.head == NULL);
    assert(list.tail == NULL);
    assert(list.size == 0);

    assert(FOS_list_push_back(&list, &value));

    out = 0;

    assert(FOS_list_pop_front(&list, &out));
    assert(out == 123);

    assert(list.head == NULL);
    assert(list.tail == NULL);
    assert(list.size == 0);

    FOS_list_free(&list);
}

/* ============================================================
 * at / at_mut / set
 * ============================================================ */

static void test_at(void)
{
    printf("test_at...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {10, 20, 30, 40};

    for (size_t i = 0; i < 4; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    for (size_t i = 0; i < 4; ++i)
    {
        const int *value = FOS_list_at(&list, i);

        assert(value != NULL);
        assert(*value == values[i]);
    }

    assert(FOS_list_at(&list, 4) == NULL);
    assert(FOS_list_at(&list, SIZE_MAX) == NULL);

    FOS_list_free(&list);
}

static void test_at_mut(void)
{
    printf("test_at_mut...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3};

    for (size_t i = 0; i < 3; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    int *value = FOS_list_at_mut(&list, 1);

    assert(value != NULL);

    *value = 99;

    int expected[] = {1, 99, 3};

    assert_int_list(&list, expected, 3);

    assert(FOS_list_at_mut(&list, 3) == NULL);
    assert(FOS_list_at_mut(&list, SIZE_MAX) == NULL);
    assert(FOS_list_at_mut(NULL, 0) == NULL);

    FOS_list_free(&list);
}

static void test_set(void)
{
    printf("test_set...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3};

    for (size_t i = 0; i < 3; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    int replacement = 100;

    assert(FOS_list_set(&list, 1, &replacement));

    int expected[] = {1, 100, 3};

    assert_int_list(&list, expected, 3);

    assert(!FOS_list_set(&list, 3, &replacement));
    assert(!FOS_list_set(&list, 0, NULL));
    assert(!FOS_list_set(NULL, 0, &replacement));

    FOS_list_free(&list);
}

/* ============================================================
 * front / back / size / empty
 * ============================================================ */

static void test_front_back(void)
{
    printf("test_front_back...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    assert(FOS_list_front(&list) == NULL);
    assert(FOS_list_back(&list) == NULL);

    int a = 10;
    int b = 20;

    assert(FOS_list_push_back(&list, &a));

    assert(*(const int *)FOS_list_front(&list) == 10);
    assert(*(const int *)FOS_list_back(&list) == 10);

    assert(FOS_list_push_back(&list, &b));

    assert(*(const int *)FOS_list_front(&list) == 10);
    assert(*(const int *)FOS_list_back(&list) == 20);

    FOS_list_free(&list);
}

static void test_null_queries(void)
{
    printf("test_null_queries...\n");

    assert(FOS_list_is_empty(NULL));
    assert(FOS_list_size(NULL) == 0);
    assert(FOS_list_front(NULL) == NULL);
    assert(FOS_list_back(NULL) == NULL);
    assert(FOS_list_at(NULL, 0) == NULL);
    assert(FOS_list_at_mut(NULL, 0) == NULL);

    FOS_list_free(NULL);
}

/* ============================================================
 * remove_at
 * ============================================================ */

static void test_remove_at(void)
{
    printf("test_remove_at...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {10, 20, 30, 40, 50};

    for (size_t i = 0; i < 5; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    /* Remove head. */
    assert(FOS_list_remove_at(&list, 0));

    {
        int expected[] = {20, 30, 40, 50};
        assert_int_list(&list, expected, 4);
        assert_list_invariants(&list);
    }

    /* Remove middle. */
    assert(FOS_list_remove_at(&list, 1));

    {
        int expected[] = {20, 40, 50};
        assert_int_list(&list, expected, 3);
        assert_list_invariants(&list);
    }

    /* Remove tail. */
    assert(FOS_list_remove_at(&list, 2));

    {
        int expected[] = {20, 40};
        assert_int_list(&list, expected, 2);
        assert_list_invariants(&list);
    }

    /* Remove last two nodes. */
    assert(FOS_list_remove_at(&list, 1));
    assert(FOS_list_remove_at(&list, 0));

    assert(FOS_list_is_empty(&list));
    assert_list_invariants(&list);

    /* Invalid positions. */
    assert(!FOS_list_remove_at(&list, 0));
    assert(!FOS_list_remove_at(&list, SIZE_MAX));
    assert(!FOS_list_remove_at(NULL, 0));

    FOS_list_free(&list);
}

/* ============================================================
 * Reverse
 * ============================================================ */

static void test_reverse(void)
{
    printf("test_reverse...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3, 4, 5};

    for (size_t i = 0; i < 5; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    assert(FOS_list_reverse(&list));

    {
        int expected[] = {5, 4, 3, 2, 1};
        assert_int_list(&list, expected, 5);
        assert_list_invariants(&list);
    }

    assert(FOS_list_reverse(&list));

    assert_int_list(&list, values, 5);
    assert_list_invariants(&list);

    FOS_list_free(&list);

    /* Empty list. */
    assert(!FOS_list_reverse(&list));
}

/* ============================================================
 * Sorting
 * ============================================================ */

static void test_sort(void)
{
    printf("test_sort...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {7, 2, 9, 1, 5, 3, 8, 4, 6};

    for (size_t i = 0; i < 9; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    assert(FOS_list_sort(&list, int_cmp));

    int expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    assert_int_list(&list, expected, 9);
    assert_list_invariants(&list);

    FOS_list_free(&list);
}

static void test_reverse_sort(void)
{
    printf("test_reverse_sort...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {7, 2, 9, 1, 5, 3};

    for (size_t i = 0; i < 6; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    assert(FOS_list_sort(&list, reverse_int_cmp));

    int expected[] = {9, 7, 5, 3, 2, 1};

    assert_int_list(&list, expected, 6);

    FOS_list_free(&list);
}

static void test_sort_duplicates(void)
{
    printf("test_sort_duplicates...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {5, 1, 3, 3, 5, 2, 1, 4};

    for (size_t i = 0; i < 8; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    assert(FOS_list_sort(&list, int_cmp));

    int expected[] = {1, 1, 2, 3, 3, 4, 5, 5};

    assert_int_list(&list, expected, 8);
    assert_list_invariants(&list);

    FOS_list_free(&list);
}

static void test_sort_stability(void)
{
    printf("test_sort_stability...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(Item)));

    Item values[] =
    {
        {2, 0},
        {1, 1},
        {2, 2},
        {1, 3},
        {2, 4},
        {1, 5}
    };

    for (size_t i = 0; i < 6; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    assert(FOS_list_sort(&list, item_cmp));

    Item expected[] =
    {
        {1, 1},
        {1, 3},
        {1, 5},
        {2, 0},
        {2, 2},
        {2, 4}
    };

    for (size_t i = 0; i < 6; ++i)
    {
        const Item *item = FOS_list_at(&list, i);

        assert(item != NULL);
        assert(item->key == expected[i].key);
        assert(item->id == expected[i].id);
    }

    assert_list_invariants(&list);

    FOS_list_free(&list);
}

static void test_sort_edge_cases(void)
{
    printf("test_sort_edge_cases...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    /*
     * Empty list.
     *
     * Your implementation deliberately returns true for a list
     * of size <= 1, provided the comparator is non-NULL.
     */
    assert(FOS_list_sort(&list, int_cmp));

    int value = 42;

    assert(FOS_list_push_back(&list, &value));

    /* One element. */
    assert(FOS_list_sort(&list, int_cmp));

    assert(*(const int *)FOS_list_front(&list) == 42);
    assert(*(const int *)FOS_list_back(&list) == 42);

    /* NULL comparator. */
    assert(!FOS_list_sort(&list, NULL));

    FOS_list_free(&list);
}

/* ============================================================
 * foreach
 * ============================================================ */

typedef struct
{
    int sum;
    size_t count;
} SumContext;

static void sum_const_item(const void *item, void *user)
{
    const int *value = item;
    SumContext *ctx = user;

    ctx->sum += *value;
    ++ctx->count;
}

static void increment_mut_item(void *item, void *user)
{
    int *value = item;
    int increment = *(int *)user;

    *value += increment;
}

static void test_foreach(void)
{
    printf("test_foreach...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3, 4, 5};

    for (size_t i = 0; i < 5; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    SumContext ctx = {0};

    FOS_list_foreach(&list, sum_const_item, &ctx);

    assert(ctx.sum == 15);
    assert(ctx.count == 5);

    FOS_list_free(&list);
}

static void test_foreach_mut(void)
{
    printf("test_foreach_mut...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {1, 2, 3, 4};

    for (size_t i = 0; i < 4; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    int increment = 10;

    FOS_list_foreach_mut(&list, increment_mut_item, &increment);

    int expected[] = {11, 12, 13, 14};

    assert_int_list(&list, expected, 4);

    FOS_list_free(&list);
}

static void test_foreach_null(void)
{
    printf("test_foreach_null...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int value = 10;
    assert(FOS_list_push_back(&list, &value));

    FOS_list_foreach(NULL, sum_const_item, NULL);
    FOS_list_foreach(&list, NULL, NULL);

    FOS_list_foreach_mut(NULL, increment_mut_item, NULL);
    FOS_list_foreach_mut(&list, NULL, NULL);

    /*
     * The list should be completely unchanged.
     */
    assert(*(const int *)FOS_list_front(&list) == 10);

    FOS_list_free(&list);
}

/* ============================================================
 * Copy semantics
 * ============================================================ */

static void test_data_is_copied(void)
{
    printf("test_data_is_copied...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int value = 123;

    assert(FOS_list_push_back(&list, &value));

    /*
     * Changing the original must not change the list.
     */
    value = 999;

    assert(*(const int *)FOS_list_front(&list) == 123);

    /*
     * Changing through at_mut must change the stored value.
     */
    int *stored = FOS_list_at_mut(&list, 0);
    assert(stored != NULL);

    *stored = 456;

    assert(*(const int *)FOS_list_front(&list) == 456);

    FOS_list_free(&list);
}

/* ============================================================
 * Large mutation test
 * ============================================================ */

static void test_many_operations(void)
{
    printf("test_many_operations...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    /*
     * Build:
     *
     * 0 1 2 ... 999
     */
    for (int i = 0; i < 1000; ++i)
    {
        assert(FOS_list_push_back(&list, &i));
        assert_list_invariants(&list);
    }

    assert(FOS_list_size(&list) == 1000);

    /*
     * Remove every other element from the front.
     */
    for (int i = 0; i < 500; ++i)
    {
        int out;

        assert(FOS_list_pop_front(&list, &out));
        assert(out == i);

        /*
         * Remove the next element from the back.
         */
        int expected_back = 999 - i;

        assert(FOS_list_pop_back(&list, &out));
        assert(out == expected_back);

        assert_list_invariants(&list);
    }

    assert(FOS_list_is_empty(&list));

    FOS_list_free(&list);
}

/* ============================================================
 * Repeated reverse/sort
 * ============================================================ */

static void test_repeated_operations(void)
{
    printf("test_repeated_operations...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int values[] = {8, 3, 7, 1, 9, 2, 6, 4, 5};

    for (size_t i = 0; i < 9; ++i)
        assert(FOS_list_push_back(&list, &values[i]));

    for (int i = 0; i < 10; ++i)
    {
        assert(FOS_list_sort(&list, int_cmp));
        assert_list_invariants(&list);

        int expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        assert_int_list(&list, expected, 9);

        assert(FOS_list_reverse(&list));
        assert_list_invariants(&list);

        int reversed[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
        assert_int_list(&list, reversed, 9);
    }

    FOS_list_free(&list);
}

/* ============================================================
 * Free
 * ============================================================ */

static void test_free(void)
{
    printf("test_free...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    for (int i = 0; i < 100; ++i)
        assert(FOS_list_push_back(&list, &i));

    FOS_list_free(&list);

    assert(list.head == NULL);
    assert(list.tail == NULL);
    assert(list.elem_size == 0);
    assert(list.size == 0);

    /*
     * Double free should be safe.
     */
    FOS_list_free(&list);

    assert(list.head == NULL);
    assert(list.tail == NULL);
    assert(list.elem_size == 0);
    assert(list.size == 0);
}

/* ============================================================
 * Invalid arguments
 * ============================================================ */

static void test_invalid_arguments(void)
{
    printf("test_invalid_arguments...\n");

    FOS_List list;
    assert(FOS_list_init(&list, sizeof(int)));

    int value = 10;

    assert(!FOS_list_push_front(NULL, &value));
    assert(!FOS_list_push_front(&list, NULL));

    assert(!FOS_list_push_back(NULL, &value));
    assert(!FOS_list_push_back(&list, NULL));

    assert(!FOS_list_pop_front(NULL, NULL));
    assert(!FOS_list_pop_back(NULL, NULL));

    assert(!FOS_list_remove_at(NULL, 0));

    assert(!FOS_list_set(NULL, 0, &value));
    assert(!FOS_list_set(&list, 0, &value));

    assert(!FOS_list_pop_front(&list, &value));
    assert(!FOS_list_pop_back(&list, &value));

    FOS_list_free(&list);
}

/* ============================================================
 * Main
 * ============================================================ */

int main(void)
{
    printf("=== FOS List Tests ===\n\n");

    test_init();
    test_invalid_init();

    test_push_front();
    test_push_back();
    test_mixed_push();

    test_pop_front();
    test_pop_front_without_output();
    test_pop_back();
    test_single_element_pop();

    test_at();
    test_at_mut();
    test_set();

    test_front_back();
    test_null_queries();

    test_remove_at();

    test_reverse();

    test_sort();
    test_reverse_sort();
    test_sort_duplicates();
    test_sort_stability();
    test_sort_edge_cases();

    test_foreach();
    test_foreach_mut();
    test_foreach_null();

    test_data_is_copied();

    test_many_operations();
    test_repeated_operations();

    test_free();
    test_invalid_arguments();

    printf("\n=== ALL FOS LIST TESTS PASSED ===\n");

    return 0;
}
