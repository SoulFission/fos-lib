#include "FOS_Queue.h"
#include <string.h>

bool FOS_queue_init(FOS_Queue *q, size_t elem_size)
{
    if (q == NULL || elem_size == 0)
        return false;

    q->elem_size = elem_size;

    return FOS_list_init(&q->queue, elem_size);
}

bool FOS_queue_enqueue(FOS_Queue *q, const void *elem)
{
    if (q == NULL || elem == NULL)
        return false;

    return FOS_list_push_back(&q->queue, elem);
}

bool FOS_queue_dequeue(FOS_Queue *q, void *elem)
{
    if (q == NULL)
        return false;

    return FOS_list_pop_front(&q->queue, elem);
}

bool FOS_queue_peek(const FOS_Queue *q, void *elem)
{
    if (q == NULL || elem == NULL || FOS_queue_is_empty(q))
        return false;

    memcpy(elem, FOS_list_front(&q->queue), q->elem_size);

    return true;
}

bool FOS_queue_is_empty(const FOS_Queue *q)
{
    if (q == NULL)
        return true;

    return FOS_list_is_empty(&q->queue);
}

size_t FOS_queue_get_size(const FOS_Queue *q)
{
    if (q == NULL)
        return 0;

    return FOS_list_size(&q->queue);
}

void FOS_queue_free(FOS_Queue *q)
{
    if (q == NULL)
        return;

    FOS_list_free(&q->queue);
}
