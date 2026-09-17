// Compilation from MSYS2 UCRT64 shell:
// gcc fos_set_ex.c -I../include -L.. -lfos -o fos_set_ex

#include "FOS_Set.h"
#include <stdio.h>

static void print_int(const void *elem, void *user)
{
    (void)user;

    printf("%d ", *(const int *)elem);
}

int main(void)
{
    FOS_Set set = FOS_set_new(sizeof(int));

    if (!FOS_set_add(&set, &(int){10}) ||
        !FOS_set_add(&set, &(int){20}) ||
        !FOS_set_add(&set, &(int){30}))
    {
        fprintf(stderr, "Failed to add an element\n");
        FOS_set_free(&set);
        return 1;
    }

    /* Adding a duplicate does not modify the set. */
    if (!FOS_set_add(&set, &(int){20}))
        printf("20 is already in the set\n");

    printf("Set size: %zu\n", FOS_set_get_size(&set));

    if (FOS_set_contains(&set, &(int){10}))
        printf("10 is present\n");

    if (!FOS_set_contains(&set, &(int){40}))
        printf("40 is not present\n");

    printf("Set elements: ");
    FOS_set_iterate(&set, print_int, NULL);
    putchar('\n');

    if (FOS_set_remove(&set, &(int){20}))
        printf("Removed 20\n");

    printf("Set size after removal: %zu\n", FOS_set_get_size(&set));

    printf("Set elements: ");
    FOS_set_iterate(&set, print_int, NULL);
    putchar('\n');

    FOS_set_free(&set);

    return 0;
}
