// Compilation from MSYS2 UCRT64 shell:
// gcc fos_stack_ex.c -I../include -L.. -lfos -o fos_stack_ex

#include "FOS_Stack.h"
#include <stdio.h>

int main(void)
{
    FOS_Stack stack;

    if (!FOS_stack_init(&stack, sizeof(int)))
    {
        fprintf(stderr, "Failed to initialize stack\n");
        return 1;
    }

    int values[] = { 10, 20, 30, 40 };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        if (!FOS_stack_push(&stack, &values[i]))
        {
            fprintf(stderr, "Failed to push element\n");
            FOS_stack_free(&stack);
            return 1;
        }
    }

    printf("Stack size: %zu\n", FOS_stack_get_size(&stack));

    int value;

    if (FOS_stack_peek(&stack, &value))
        printf("Top element: %d\n", value);

    printf("Popping elements:\n");

    while (!FOS_stack_is_empty(&stack))
    {
        if (FOS_stack_pop(&stack, &value))
            printf("%d\n", value);
    }

    printf("Stack size: %zu\n", FOS_stack_get_size(&stack));

    FOS_stack_free(&stack);

    return 0;
}
