#ifndef FOS_LIST_H
#define FOS_LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct FOS_ListNode {
    void *data;
    struct FOS_ListNode *next;
} FOS_ListNode;

typedef struct {
    FOS_ListNode *head;
    FOS_ListNode *tail;
    size_t size;
    size_t elem_size;
} FOS_List;

bool FOS_list_init(FOS_List *list, size_t elem_size);
bool FOS_list_push_front(FOS_List *list, const void *item);
bool FOS_list_pop_front(FOS_List *list, void *out);
bool FOS_list_is_empty(const FOS_List *list);
void FOS_list_free(FOS_List *list);
bool FOS_list_push_back(FOS_List *list, const void *item);
bool FOS_list_pop_back(FOS_List *list, void *out);
const void *FOS_list_at(const FOS_List *list, size_t pos);
void *FOS_list_at_mut(FOS_List *list, size_t pos);
bool FOS_list_set(FOS_List *list, size_t pos, const void *value);
const void *FOS_list_front(const FOS_List *list);
const void *FOS_list_back(const FOS_List *list);
size_t FOS_list_size(const FOS_List *list);
bool FOS_list_reverse(FOS_List *list);
bool FOS_list_sort(FOS_List *list, int (*cmp_func)(const void *val1, const void *val2));
void FOS_list_foreach(const FOS_List *list, void (*func)(const void *item, void *user), void *user);
void FOS_list_foreach_mut(FOS_List *list, void (*func)(void *item, void *user), void *user);

// Comparison function example for `FOS_list_sort()` using int
// Optional: could use `return (a > b) - (a < b);` instead of branching
static inline int FOS_list_cmp_int(const void *val1, const void *val2)
{
    int a = *(const int *)val1;
    int b = *(const int *)val2;

    if (a < b)
        return -1;
    else if (a > b)
        return 1;
    else
        return 0;
}

#endif
