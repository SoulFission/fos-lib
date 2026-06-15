#ifndef FOS_QUEUE_H
#define FOS_QUEUE_H

#include "FOS_List.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    FOS_List queue;
    size_t elem_size;
} FOS_Queue;

bool FOS_queue_init(FOS_Queue *q, size_t elem_size);
bool FOS_queue_enqueue(FOS_Queue *q, const void *elem);
bool FOS_queue_dequeue(FOS_Queue *q, void *elem);
bool FOS_queue_peek(const FOS_Queue *q, void *elem);
bool FOS_queue_is_empty(const FOS_Queue *q);
size_t FOS_queue_get_size(const FOS_Queue *q);
void FOS_queue_free(FOS_Queue *q);

#endif
