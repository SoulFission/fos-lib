#include "FOS_Stack.h"
#include <stdio.h>
#include <assert.h>

int main(void)
{
    FOS_Stack s = { 0 };

    assert(FOS_stack_init(&s, sizeof(double)));

    assert(FOS_stack_is_empty(&s));

    assert(FOS_stack_push(&s, &(double) { 3.59 }));
    assert(FOS_stack_push(&s, &(double) { 2.181 }));
    assert(FOS_stack_push(&s, &(double) { 8.44 }));

    assert(FOS_stack_get_size(&s) == 3ULL);

    double val;

    assert(FOS_stack_pop(&s, &val));

    printf("val = %f\n", val);

    double val2;

    assert(FOS_stack_peek(&s, &val2));

    printf("val2 = %f\n", val2);

    for (size_t i = 0; i < 3000; ++i)
        assert(FOS_stack_push(&s, &(double) { (double)i }));

    double val3;

    assert(FOS_stack_peek(&s, &val3));

    printf("val2 = %f\n", val3); 

    FOS_stack_free(&s);  

    printf("All test passed.\n");

    return 0;
}