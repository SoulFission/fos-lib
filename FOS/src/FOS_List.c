#include "FOS_List.h"
#include "FOS_Memory.h"
#include <string.h>

static FOS_ListNode *FOS_list_merge_sorted(FOS_ListNode *left, FOS_ListNode *right, int (*cmp_func)(const void *, const void *));
static FOS_ListNode *FOS_list_merge_sort_impl(FOS_ListNode *head, int (*cmp_func)(const void *, const void *));

bool FOS_list_init(FOS_List *list, size_t elem_size)
{
    if (list == NULL || elem_size == 0)
        return false;

    list->head = NULL;
    list->tail = NULL;
    list->elem_size = elem_size;
    list->size = 0;

    return true;
}

bool FOS_list_push_front(FOS_List *list, const void *item)
{
    if (list == NULL || item == NULL || list->elem_size == 0)
        return false;

    FOS_ListNode *node = FOS_alloc(sizeof(*node));

    if (node == NULL)
        return false;

    node->data = FOS_alloc(list->elem_size);

    if (node->data == NULL)
    {
        FOS_free(node);

        return false;
    }
    
    memcpy(node->data, item, list->elem_size);

    node->next = NULL;

    if (list->head == NULL)
    {
        list->head = node;
        list->tail = node;
    }
    else
    {
        node->next = list->head;
        list->head = node;
    }

    ++list->size;

    return true;
}

bool FOS_list_pop_front(FOS_List *list, void *out)
{
    if (list == NULL || list->head == NULL || list->size == 0)
        return false;

    FOS_ListNode *temp = list->head;

    if (out != NULL)
        memcpy(out, temp->data, list->elem_size);

    list->head = temp->next;

    if (list->head == NULL)
        list->tail = NULL;

    FOS_free(temp->data);
    FOS_free(temp);

    --list->size;

    return true;
}

bool FOS_list_is_empty(const FOS_List *list)
{
    return list == NULL || list->size == 0;
}

void FOS_list_free(FOS_List *list)
{
    if (list == NULL)
        return;

    FOS_ListNode *current = list->head;

    while (current != NULL)
    {
        FOS_ListNode *next = current->next;

        FOS_free(current->data);
        FOS_free(current);

        current = next;
    }

    list->elem_size = 0;
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

bool FOS_list_push_back(FOS_List *list, const void *item)
{
    if (list == NULL || item == NULL || list->elem_size == 0)
        return false;

    if (list->head == NULL)
        return FOS_list_push_front(list, item);

    FOS_ListNode *node = FOS_alloc(sizeof(*node));

    if (node == NULL)
        return false;

    node->data = FOS_alloc(list->elem_size);

    if (node->data == NULL)
    {
        FOS_free(node);

        return false;
    }

    memcpy(node->data, item, list->elem_size);

    node->next = NULL;

    list->tail->next = node;
    list->tail = node;
    ++list->size;

    return true;
}

bool FOS_list_pop_back(FOS_List *list, void *out)
{
    if (list == NULL || list->head == NULL)
        return false;

    FOS_ListNode *current = list->head;

    if (current == list->tail)
    {
        if (out != NULL)
            memcpy(out, current->data, list->elem_size);

        FOS_free(current->data);
        FOS_free(current);

        list->head = NULL;
        list->tail = NULL;
        list->size = 0;

        return true;
    }

    while (current->next != list->tail)
        current = current->next;

    if (out != NULL)
        memcpy(out, list->tail->data, list->elem_size);

    FOS_free(list->tail->data);
    FOS_free(list->tail);

    current->next = NULL;
    list->tail = current;
    --list->size;

    return true;
}


const void *FOS_list_at(const FOS_List *list, size_t pos)
{
    if (list == NULL || list->head == NULL || list->elem_size == 0)
        return NULL;

    const FOS_ListNode *current = list->head;
    size_t i = 0;

    while (current != NULL)
    {
        if (i == pos)
            return current->data;

        current = current->next;
        ++i;
    }

    return NULL;
}

void *FOS_list_at_mut(FOS_List *list, size_t pos)
{
    if (list == NULL)
        return NULL;

    FOS_ListNode *current = list->head;

    for (size_t i = 0; current != NULL && i < pos; ++i)
        current = current->next;

    return current ? current->data : NULL;
}

bool FOS_list_set(FOS_List *list, size_t pos, const void *value)
{
    void *ptr = FOS_list_at_mut(list, pos);

    if (ptr == NULL || value == NULL)
        return false;

    memcpy(ptr, value, list->elem_size);

    return true;
}

const void *FOS_list_front(const FOS_List *list)
{
    if (list == NULL || list->head == NULL)
        return NULL;

    return list->head->data;
}

const void *FOS_list_back(const FOS_List *list)
{
    if (list == NULL || list->tail == NULL)
        return NULL;

    return list->tail->data;
}

size_t FOS_list_size(const FOS_List *list)
{
    if (list == NULL)
        return 0;

    return list->size;
}

bool FOS_list_reverse(FOS_List *list)
{
    if (list == NULL || list->head == NULL)
        return false;

    FOS_ListNode *old_head = list->head;
    FOS_ListNode *current = list->head;
    FOS_ListNode *prev = NULL;

    while (current != NULL)
    {
        FOS_ListNode *next = current->next;

        current->next = prev;
        prev = current;
        current = next;
    }

    list->head = prev;
    list->tail = old_head;

    return true;
}

bool FOS_list_sort(FOS_List *list, int (*cmp_func)(const void *val1, const void *val2))
{
    if (list == NULL || cmp_func == NULL)
        return false;

    if (list->size <= 1)
        return true;

    /* Sort the list. */
    list->head = FOS_list_merge_sort_impl(list->head, cmp_func);

    /*
     * Recompute tail, since node order changed.
     */
    FOS_ListNode *current = list->head;

    while (current->next != NULL)
        current = current->next;

    list->tail = current;

    return true;
}

void FOS_list_foreach(const FOS_List *list, void (*func)(const void *item, void *user), void *user)
{
    if (list == NULL || func == NULL)
        return;

    FOS_ListNode *current = list->head;

    while (current != NULL)
    {
        func(current->data, user);
        current = current->next;
    }
}

void FOS_list_foreach_mut(FOS_List *list, void (*func)(void *item, void *user), void *user)
{
    if (list == NULL || func == NULL)
        return;

    FOS_ListNode *current = list->head;

    while (current != NULL)
    {
        func(current->data, user);
        current = current->next;
    }
}

/*
 * Merge two already sorted linked lists.
 *
 * This function does not allocate memory.
 * It simply rewires next pointers.
 */
static FOS_ListNode *FOS_list_merge_sorted(FOS_ListNode *left, FOS_ListNode *right, int (*cmp_func)(const void *, const void *))
{
    /* Dummy node simplifies handling the head. */
    FOS_ListNode dummy;
    FOS_ListNode *tail = &dummy;

    dummy.next = NULL;

    while (left != NULL && right != NULL)
    {
        /*
         * Stable sorting:
         * if elements compare equal,
         * take from the left list first.
         */
        if (cmp_func(left->data, right->data) <= 0)
        {
            tail->next = left;
            left = left->next;
        }
        else
        {
            tail->next = right;
            right = right->next;
        }

        tail = tail->next;
    }

    /* Attach the remaining nodes. */
    tail->next = (left != NULL) ? left : right;

    return dummy.next;
}

/*
 * Recursive merge sort implementation.
 */
static FOS_ListNode *FOS_list_merge_sort_impl(FOS_ListNode *head, int (*cmp_func)(const void *, const void *))
{
    /* Empty or single-node list is already sorted. */
    if (head == NULL || head->next == NULL)
        return head;

    /*
     * Split the list into two halves using
     * the classic slow/fast pointer technique.
     */
    FOS_ListNode *slow = head;
    FOS_ListNode *fast = head->next;

    while (fast != NULL && fast->next != NULL)
    {
        slow = slow->next;
        fast = fast->next->next;
    }

    /*
     * 'slow' now points to the end of
     * the left half.
     */
    FOS_ListNode *right = slow->next;
    slow->next = NULL;

    FOS_ListNode *left = head;

    /* Recursively sort both halves. */
    left = FOS_list_merge_sort_impl(left, cmp_func);
    right = FOS_list_merge_sort_impl(right, cmp_func);

    /* Merge them back together. */
    return FOS_list_merge_sorted(left, right, cmp_func);
}
