// Compilation from MSYS2 UCRT64 shell:
// gcc fos_heap_ex.c -I../include -L.. -lfos -o fos_heap_ex

#include "FOS_Heap.h"
#include <stdio.h>

static int compare_ints(const void *fst, const void *snd)
{
    const int *a = fst;
    const int *b = snd;

    if (*a < *b)
        return -1;

    if (*a > *b)
        return 1;

    return 0;
}

int main(void)
{
    FOS_Heap heap;

    if (!FOS_heap_init(&heap, compare_ints, sizeof(int)))
    {
        fprintf(stderr, "Failed to initialize heap\n");
        return 1;
    }

    int values[] = { 7, 2, 9, 1, 5, 3 };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
        FOS_heap_push(&heap, &values[i]);

    int value;

    if (FOS_heap_peek(&heap, &value))
        printf("Minimum: %d\n", value);

    printf("Elements in priority order:\n");

    while (!FOS_heap_is_empty(&heap))
    {
        FOS_heap_pop(&heap, &value);
        printf("%d\n", value);
    }

    FOS_heap_free(&heap);

    return 0;
}
