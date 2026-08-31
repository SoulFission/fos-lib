#include "FOS_Stack.h"
#include <string.h>

bool FOS_stack_init(FOS_Stack *s, size_t elem_size)
{
    if (s == NULL || elem_size == 0)
        return false;

    s->elem_size = elem_size;

    return FOS_list_init(&s->stack, elem_size);
}

bool FOS_stack_push(FOS_Stack *s, const void *elem)
{
    if (s == NULL || elem == NULL)
        return false;

    return FOS_list_push_front(&s->stack, elem);
}

bool FOS_stack_pop(FOS_Stack *s, void *elem)
{
    if (s == NULL)
        return false;

    return FOS_list_pop_front(&s->stack, elem);
}

bool FOS_stack_peek(const FOS_Stack *s, void *elem)
{
    if (s == NULL || elem == NULL || FOS_stack_is_empty(s))
        return false;

    memcpy(elem, FOS_list_front(&s->stack), s->elem_size);

    return true;
}

bool FOS_stack_is_empty(const FOS_Stack *s)
{
    if (s == NULL)
        return true;

    return FOS_list_is_empty(&s->stack);
}

size_t FOS_stack_get_size(const FOS_Stack *s)
{
    if (s == NULL)
        return 0;

    return FOS_list_size(&s->stack);
}

void FOS_stack_free(FOS_Stack *s)
{
    if (s == NULL)
        return;

    FOS_list_free(&s->stack);
}
