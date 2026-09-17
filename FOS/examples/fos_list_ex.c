// Compilation from MSYS2 UCRT64 shell:
// gcc fos_list_ex.c -I../include -L.. -lfos -o fos_list_ex

#include "FOS_List.h"
#include <stdio.h>

static void print_int(const void *item, void *user)
{
    (void)user;

    printf("%d ", *(const int *)item);
}

static void increment_int(void *item, void *user)
{
    int amount = *(int *)user;

    *(int *)item += amount;
}

int main(void)
{
    FOS_List list;

    if (!FOS_list_init(&list, sizeof(int)))
    {
        fprintf(stderr, "Failed to initialize list\n");
        return 1;
    }

    int values[] = { 7, 2, 9, 1, 5 };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        if (!FOS_list_push_back(&list, &values[i]))
        {
            fprintf(stderr, "Failed to add element\n");
            FOS_list_free(&list);
            return 1;
        }
    }

    printf("Original list: ");
    FOS_list_foreach(&list, print_int, NULL);
    putchar('\n');

    int value;

    if (FOS_list_pop_front(&list, &value))
        printf("Removed from front: %d\n", value);

    if (FOS_list_pop_back(&list, &value))
        printf("Removed from back: %d\n", value);

    printf("After removing both ends: ");
    FOS_list_foreach(&list, print_int, NULL);
    putchar('\n');

    int increment = 10;
    FOS_list_foreach_mut(&list, increment_int, &increment);

    printf("After adding 10 to each element: ");
    FOS_list_foreach(&list, print_int, NULL);
    putchar('\n');

    if (FOS_list_sort(&list, FOS_list_cmp_int))
    {
        printf("Sorted list: ");
        FOS_list_foreach(&list, print_int, NULL);
        putchar('\n');
    }

    FOS_list_reverse(&list);

    printf("Reversed list: ");
    FOS_list_foreach(&list, print_int, NULL);
    putchar('\n');

    const int *front = FOS_list_front(&list);
    const int *back = FOS_list_back(&list);

    if (front != NULL && back != NULL)
        printf("Front: %d, Back: %d\n", *front, *back);

    printf("List size: %zu\n", FOS_list_size(&list));

    FOS_list_free(&list);

    return 0;
}
