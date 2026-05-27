#ifndef FOS_STACK_H
#define FOS_STACK_H

#include "FOS_List.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    FOS_List stack;
    size_t elem_size;
} FOS_Stack;

bool FOS_stack_init(FOS_Stack *s, size_t elem_size);
bool FOS_stack_push(FOS_Stack *s, const void *elem);
bool FOS_stack_pop(FOS_Stack *s, void *elem);
bool FOS_stack_peek(const FOS_Stack *s, void *elem);
bool FOS_stack_is_empty(const FOS_Stack *s);
size_t FOS_stack_get_size(const FOS_Stack *s);
void FOS_stack_free(FOS_Stack *s);

#endif
