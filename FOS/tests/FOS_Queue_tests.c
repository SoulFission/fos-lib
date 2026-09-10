#include <stdio.h>
#include <assert.h>
#include "FOS_Queue.h"

int main(void)
{
    FOS_Queue q = { 0 };

    assert(FOS_queue_init(&q, sizeof(int)));

    assert(FOS_queue_is_empty(&q));

    assert(FOS_queue_enqueue(&q, &(int) { 3 }));
    assert(FOS_queue_enqueue(&q, &(int) { 5 }));
    assert(FOS_queue_enqueue(&q, &(int) { 9 }));

    assert(!FOS_queue_is_empty(&q));

    assert(FOS_queue_get_size(&q) == 3u);

    int val;

    FOS_queue_peek(&q, &val);

    assert(val == 3);

    int fst;

    assert(FOS_queue_dequeue(&q, &fst));

    assert(fst == 3);

    int snd;

    assert(FOS_queue_dequeue(&q, &snd));

    assert(snd == 5);

    int trd;

    assert(FOS_queue_dequeue(&q, &trd));

    assert(trd == 9);

    assert(FOS_queue_is_empty(&q));

    FOS_queue_free(&q);

    printf("All tests passed");

    return 0;
}