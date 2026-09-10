#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "FOS_List.h"

int main(void)
{
    FOS_List list;

    assert(FOS_list_init(&list, sizeof(int)));

    assert(FOS_list_push_front(&list, &(int) { 3 }));

    assert(!FOS_list_is_empty(&list));

    assert(FOS_list_push_front(&list, &(int) { 5 }));

    int val;

    assert(FOS_list_pop_back(&list, &val));

    assert(val == 3);

    assert(FOS_list_push_back(&list, &(int) { 9 }));

    int val2;

    assert(FOS_list_pop_front(&list, &val2));

    assert(val2 == 5);

    assert(FOS_list_push_back(&list, &(int) { 8 }));
    assert(FOS_list_push_back(&list, &(int) { 7 }));

    const int *elem = FOS_list_at(&list, 1u);

    assert(*elem == 8);

    assert(FOS_list_remove_at(&list, 1u));

    assert(FOS_list_size(&list) == 2u);

    const int *front, *back;

    assert(*(front = FOS_list_front(&list)) == 9);
    assert(*(back = FOS_list_back(&list)) == 7);

    assert(FOS_list_push_back(&list, &(int) { 11 }));
    assert(FOS_list_push_back(&list, &(int) { 4 }));

    assert(FOS_list_sort(&list, FOS_list_cmp_int));

    FOS_List list2 = { 0 };

    assert(FOS_list_init(&list2, sizeof(int)));

    void print_fill_rev(const void *item, void *user);
    void print_elem(const void *item, void *user);

    puts("List contents:");
    FOS_list_foreach(&list, print_fill_rev, &list2);

    puts("Reversed list contents:");
    FOS_list_foreach(&list2, print_elem, NULL);

    assert(FOS_list_set(&list, 0, &(int) { 3 }));

    *(int *)FOS_list_at_mut(&list, 1u) = 5;

    puts("Mutated list contents:");

    FOS_list_foreach(&list, print_elem, NULL);

    assert(FOS_list_reverse(&list));

    puts("Reversed:");

    FOS_list_foreach(&list, print_elem, NULL);

    FOS_list_free(&list);
    FOS_list_free(&list2);

    puts("All tests passed.");

    return 0;
}

void print_fill_rev(const void *item, void *user)
{
    FOS_List *rev = user;
    const int val = *(const int *)item;

    assert(FOS_list_push_front(rev, &val));

    printf("%d\n", val);
}

void print_elem(const void *item, void *user)
{
    (void)user;

    const int val = *(const int *)item;

    printf("%d\n", val);
}
