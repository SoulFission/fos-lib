#include "FOS_Set.h"
#include <stdio.h>
#include <assert.h>

void print(const void *elem, void *user)
{
    (void)user;
    int num = *(int *)elem;

    printf("%d ", num);
}

int main(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    assert(FOS_set_add(&set, &(int) { 3 }));
    assert(FOS_set_add(&set, &(int) { 5 }));
    assert(FOS_set_add(&set, &(int) { 9 }));

    assert(!FOS_set_add(&set, &(int) { 5 }));

    assert(FOS_set_get_size(&set) == 3ULL);

    assert(FOS_set_contains(&set, &(int) { 9 }));

    assert(FOS_set_remove(&set, &(int) { 3 }));

    assert(FOS_set_get_size(&set) == 2ULL);

    assert(FOS_set_add(&set, &(int) { 7 }));

    assert(FOS_set_add(&set, &(int) { 11 }));

    FOS_set_iterate(&set, print, NULL);

    printf("\nAll tests passed.\n");

    FOS_set_free(&set);

    return 0;
}