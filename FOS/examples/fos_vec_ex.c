// Compilation from MSYS2 UCRT64 shell:
// gcc fos_string_ex.c -I../include -L.. -lfos -o fos_string_ex

#include "FOS_Vec.h"
#include <stdio.h>

static int compare_ints(const void *a, const void *b)
{
    const int *x = a;
    const int *y = b;

    if (*x < *y)
        return -1;

    if (*x > *y)
        return 1;

    return 0;
}

static void print_vector(const FOS_Vec *vec)
{
    printf("[");

    FOS_VEC_FOR_EACH(const int, vec, it)
    {
        if (it != (const int *)vec->data)
            printf(", ");

        printf("%d", *it);
    }

    printf("]\n");
}

/*
 * Create a type-specific interface for vectors of int.
 */
FOS_VEC_DEFINE(int, int)

int main(void)
{
    FOS_Vec vec = FOS_vec_new(sizeof(int));

    if (vec.data == NULL)
    {
        fprintf(stderr, "Failed to create vector\n");
        return 1;
    }

    printf("Initial size: %zu\n", vec.size);
    printf("Initial capacity: %zu\n", vec.capacity);

    /*
     * Add several elements to the end of the vector.
     */
    int values[] = { 40, 10, 50, 20, 30 };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        if (!FOS_vec_push(&vec, &values[i]))
        {
            fprintf(stderr, "Failed to push element\n");
            FOS_vec_free(&vec);
            return 1;
        }
    }

    printf("\nAfter push:\n");
    print_vector(&vec);

    printf("Size: %zu\n", vec.size);
    printf("Capacity: %zu\n", vec.capacity);

    /*
     * Access individual elements with FOS_vec_at().
     */
    int *element = FOS_vec_at(&vec, 2);

    if (element != NULL)
        printf("Element at index 2: %d\n", *element);

    /*
     * Modify an element directly through the returned pointer.
     */
    element = FOS_vec_at(&vec, 2);

    if (element != NULL)
        *element = 55;

    printf("After modifying index 2:\n");
    print_vector(&vec);

    /*
     * Insert an element before an existing position.
     */
    int inserted = 25;

    if (FOS_vec_insert(&vec, 3, &inserted))
    {
        printf("After inserting 25 at index 3:\n");
        print_vector(&vec);
    }

    /*
     * Remove one element while preserving the order
     * of the remaining elements.
     */
    if (FOS_vec_erase_at(&vec, 1))
    {
        printf("After ordered erase at index 1:\n");
        print_vector(&vec);
    }

    /*
     * Unordered removal is useful when preserving order is
     * unnecessary. The final element replaces the removed one.
     */
    if (FOS_vec_erase_unordered(&vec, 0))
    {
        printf("After unordered erase at index 0:\n");
        print_vector(&vec);
    }

    /*
     * Insert several elements again so that the vector
     * contains enough data for the following operations.
     */
    int extra[] = { 60, 15, 35 };

    for (size_t i = 0; i < sizeof(extra) / sizeof(extra[0]); ++i)
    {
        if (!FOS_vec_push(&vec, &extra[i]))
        {
            fprintf(stderr, "Failed to add extra element\n");
            FOS_vec_free(&vec);
            return 1;
        }
    }

    printf("\nBefore sorting:\n");
    print_vector(&vec);

    /*
     * Sort the logical contents of the vector.
     */
    if (FOS_vec_sort(&vec, compare_ints))
    {
        printf("After sorting:\n");
        print_vector(&vec);
    }

    /*
     * Traverse the vector in reverse order.
     */
    printf("Reverse traversal: ");

    FOS_VEC_FOR_EACH_REV(const int, &vec, it)
    {
        printf("%d ", *it);
    }

    putchar('\n');

    /*
     * FOS_VEC_FOR_EACH_RAW() gives access to each element
     * as a sequence of bytes.
     */
    printf("Raw first bytes of each element: ");

    FOS_VEC_FOR_EACH_RAW(&vec, raw)
    {
        unsigned char first_byte = (unsigned char)raw[0];
        printf("%02X ", first_byte);
    }

    putchar('\n');

    /*
     * Reserve storage for at least 32 elements.
     * This changes capacity but not logical size.
     */
    if (FOS_vec_reserve(&vec, 32))
    {
        printf("Capacity after reserve: %zu\n", vec.capacity);
        printf("Size after reserve: %zu\n", vec.size);
    }

    /*
     * Resize to create additional zero-initialized elements.
     */
    size_t old_size = vec.size;

    if (FOS_vec_resize(&vec, old_size + 3))
    {
        printf("\nAfter resize:\n");
        print_vector(&vec);

        printf("The three new elements are zero-initialized.\n");
    }

    /*
     * Modify the newly created elements.
     */
    for (size_t i = old_size; i < vec.size; ++i)
    {
        int *value = FOS_vec_at(&vec, i);

        if (value != NULL)
            *value = (int)((i + 1) * 100);
    }

    printf("After filling the new elements:\n");
    print_vector(&vec);

    /*
     * Remove an inclusive range.
     */
    if (vec.size >= 4 && FOS_vec_erase_range(&vec, 1, 3))
    {
        printf("After erasing indices 1 through 3:\n");
        print_vector(&vec);
    }

    /*
     * Pop the last element.
     */
    int last;

    if (FOS_vec_pop(&vec, &last))
        printf("Popped value: %d\n", last);

    printf("After pop:\n");
    print_vector(&vec);

    /*
     * Copy the vector into another vector.
     */
    FOS_Vec copy = FOS_vec_new(sizeof(int));

    if (copy.data == NULL)
    {
        fprintf(stderr, "Failed to create copy vector\n");
        FOS_vec_free(&vec);
        return 1;
    }

    if (FOS_vec_copy(&copy, &vec))
    {
        printf("\nCopied vector:\n");
        print_vector(&copy);

        printf("Copied vector size: %zu\n", copy.size);
        printf("Copied vector capacity: %zu\n", copy.capacity);
    }

    /*
     * FOS_vec_copy() creates independent vector storage.
     * Modifying the copy does not modify the original.
     */
    int *copy_first = FOS_vec_at(&copy, 0);

    if (copy_first != NULL)
        *copy_first = -1;

    printf("Modified copy:\n");
    print_vector(&copy);

    printf("Original vector remains:\n");
    print_vector(&vec);

    /*
     * shrink_to_fit() removes excess capacity.
     */
    if (FOS_vec_shrink_to_fit(&copy))
    {
        printf("Copy capacity after shrink_to_fit: %zu\n",
               copy.capacity);
    }

    /*
     * Use the type-specific interface created by FOS_VEC_DEFINE().
     */
    FOS_Vec_int typed_vec = FOS_vec_int_new();

    if (typed_vec.data == NULL)
    {
        fprintf(stderr, "Failed to create typed vector\n");
        FOS_vec_free(&copy);
        FOS_vec_free(&vec);
        return 1;
    }

    if (!FOS_vec_int_push(&typed_vec, 100) ||
        !FOS_vec_int_push(&typed_vec, 200) ||
        !FOS_vec_int_push(&typed_vec, 300))
    {
        fprintf(stderr, "Failed to populate typed vector\n");
        FOS_vec_int_free(&typed_vec);
        FOS_vec_free(&copy);
        FOS_vec_free(&vec);
        return 1;
    }

    printf("\nTyped vector:\n");

    FOS_VEC_FOR_EACH(const int, &typed_vec, it)
    {
        printf("%d ", *it);
    }

    putchar('\n');

    /*
     * Access the typed vector through its generated wrappers.
     */
    int *typed_element = FOS_vec_int_at(&typed_vec, 1);

    if (typed_element != NULL)
        printf("Typed vector element at index 1: %d\n", *typed_element);

    /*
     * Pop from the typed vector.
     */
    int popped;

    if (FOS_vec_int_pop(&typed_vec, &popped))
        printf("Popped from typed vector: %d\n", popped);

    /*
     * Clear removes all logical elements but retains capacity.
     */
    FOS_vec_clear(&vec);

    printf("\nAfter clearing original vector:\n");
    printf("Size: %zu\n", vec.size);
    printf("Capacity: %zu\n", vec.capacity);

    /*
     * Free all vectors.
     */
    FOS_vec_int_free(&typed_vec);
    FOS_vec_free(&copy);
    FOS_vec_free(&vec);

    return 0;
}
