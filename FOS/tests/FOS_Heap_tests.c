#include <stdio.h>
#include <assert.h>
#include "FOS_Heap.h"

int cmp(const void *fst, const void *snd)
{
    const int a = *(const int *)fst;
    const int b = *(const int *)snd;

    return (a > b) - (b > a);
}

int main(void)
{
    FOS_Heap h = { 0 };

    assert(FOS_heap_init(&h, cmp, sizeof(int)));

    assert(FOS_heap_is_empty(&h));

    assert(FOS_heap_push(&h, &(int) { 9 }));
    assert(FOS_heap_push(&h, &(int) { 5 }));
    assert(FOS_heap_push(&h, &(int) { 3 }));
    assert(FOS_heap_push(&h, &(int) { 4 }));

    assert(FOS_heap_get_size(&h) == 4);

    int top;

    assert(FOS_heap_peek(&h, &top));
    assert(top == 3);

    int val;

    assert(FOS_heap_pop(&h, &val));
    assert(val == 3);

    assert(FOS_heap_get_size(&h) == 3);

    int new_top;

    assert(FOS_heap_peek(&h, &new_top));
    assert(new_top == 4);

    FOS_heap_free(&h);

    printf("All tests passed.");

    return 0;
}