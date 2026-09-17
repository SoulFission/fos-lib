// Compilation from MSYS2 UCRT64 shell:
// gcc fos_queue_ex.c -I../include -L.. -lfos -o fos_queue_ex

#include "FOS_Queue.h"
#include <stdio.h>

int main(void)
{
    FOS_Queue queue;

    if (!FOS_queue_init(&queue, sizeof(int)))
    {
        fprintf(stderr, "Failed to initialize queue\n");
        return 1;
    }

    int values[] = { 10, 20, 30, 40 };

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        if (!FOS_queue_enqueue(&queue, &values[i]))
        {
            fprintf(stderr, "Failed to enqueue element\n");
            FOS_queue_free(&queue);
            return 1;
        }
    }

    printf("Queue size: %zu\n", FOS_queue_get_size(&queue));

    int value;

    if (FOS_queue_peek(&queue, &value))
        printf("Front element: %d\n", value);

    printf("Dequeuing elements:\n");

    while (!FOS_queue_is_empty(&queue))
    {
        if (FOS_queue_dequeue(&queue, &value))
            printf("%d\n", value);
    }

    printf("Queue size: %zu\n", FOS_queue_get_size(&queue));

    FOS_queue_free(&queue);

    return 0;
}
