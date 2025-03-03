#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* =============================================================================
 * q_new - Create an empty queue (sentinel node)
 * =============================================================================
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}


/* =============================================================================
 * q_free - Free all storage used by queue, including the sentinel.
 * =============================================================================
 */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *elem, *safe = NULL;
    list_for_each_entry_safe (elem, safe, head, list)
        q_release_element(elem);
    free(head);
}

/* =============================================================================
 * q_insert_head - Insert an element at the head (LIFO)
 * =============================================================================
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_elem = malloc(sizeof(element_t));
    if (!new_elem)
        return false;
    new_elem->value = strdup(s);
    if (!new_elem->value) {
        free(new_elem);
        return false;
    }
    list_add(&new_elem->list, head);
    return true;
}

/* =============================================================================
 * q_insert_tail - Insert an element at the tail (FIFO)
 * =============================================================================
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_elem = malloc(sizeof(element_t));
    if (!new_elem)
        return false;
    new_elem->value = strdup(s);
    if (!new_elem->value) {
        free(new_elem);
        return false;
    }
    list_add_tail(&new_elem->list, head);
    return true;
}

/* =============================================================================
 * q_remove_head - Remove an element from head of queue.
 *                If sp is non-NULL, copy the element’s string into sp (up to
 *                bufsize-1 characters) and NUL-terminate it.
 *                Do not free the element; return it for the caller to release.
 * =============================================================================
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *node = head->next;
    element_t *elem = container_of(node, element_t, list);
    list_del(node);
    if (sp) {
        strncpy(sp, elem->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return elem;
}

/* =============================================================================
 * q_remove_tail - Remove an element from tail of queue.
 *                If sp is non-NULL, copy the element’s string into sp (up to
 *                bufsize-1 characters) and NUL-terminate it.
 * =============================================================================
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *node = head->prev;
    element_t *elem = container_of(node, element_t, list);
    list_del(node);
    if (sp) {
        strncpy(sp, elem->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return elem;
}

/* =============================================================================
 * q_size - Return the number of elements in the queue.
 * =============================================================================
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int count = 0;
    struct list_head *cur;
    list_for_each (cur, head)
        count++;
    return count;
}

/* =============================================================================
 * q_delete_mid - Delete the middle node in queue.
 *                The middle node is defined as floor(n/2) (0-indexed).
 *                Do nothing if the queue is NULL or empty.
 * =============================================================================
 */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head) || head->next->next == head)
        return false;
    struct list_head *slow = head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    list_del(slow);
    element_t *mid = container_of(slow, element_t, list);
    free(mid->value);
    free(mid);
    return true;
}

/* =============================================================================
 * q_delete_dup - Delete all nodes that have duplicate strings,
 *                leaving only distinct strings.
 *                The queue is assumed to be sorted.
 * =============================================================================
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    bool removed = false;
    struct list_head *cur = head->next;
    while (cur != head) {
        const element_t *cur_elem = container_of(cur, element_t, list);
        bool dup = false;
        /* 檢查下一個節點是否與當前節點重複 */
        while (cur->next != head) {
            const element_t *next_elem =
                container_of(cur->next, element_t, list);
            if (strcmp(cur_elem->value, next_elem->value) == 0) {
                dup = true;
                break;
            } else {
                break;
            }
        }
        struct list_head *next_cur = cur->next;
        if (dup) {
            const char *dup_val = cur_elem->value;
            /* 移除所有連續重複的節點 */
            while (cur != head &&
                   strcmp(container_of(cur, element_t, list)->value, dup_val) ==
                       0) {
                struct list_head *tmp = cur->next;
                list_del(cur);
                q_release_element(container_of(cur, element_t, list));
                removed = true;
                cur = tmp;
            }
        } else {
            cur = next_cur;
        }
    }
    return removed;
}

/* =============================================================================
 * q_swap - Swap every two adjacent nodes.
 *          This implementation swaps the stored string pointers.
 * =============================================================================
 */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur = head->next;
    while (cur != head && cur->next != head) {
        element_t *first = container_of(cur, element_t, list);
        element_t *second = container_of(cur->next, element_t, list);
        char *temp = first->value;
        first->value = second->value;
        second->value = temp;
        cur = cur->next->next;
    }
}

/* =============================================================================
 * q_reverse - Reverse the elements in the queue in-place.
 *             Only adjust pointers; do not allocate or free nodes.
 * =============================================================================
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur = head;
    do {
        struct list_head *temp = cur->next;
        cur->next = cur->prev;
        cur->prev = temp;
        cur = temp;
    } while (cur != head);
}

/* =============================================================================
 * q_reverseK - Reverse the nodes of the list k at a time.
 *              If the remaining nodes are fewer than k, leave them as is.
 * =============================================================================
 */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || k <= 1)
        return;
    struct list_head *cur = head->next;
    while (cur != head) {
        struct list_head *temp = cur;
        int count = 0;
        while (temp != head && count < k) {
            temp = temp->next;
            count++;
        }
        if (count < k)
            break;
        struct list_head **group = malloc(k * sizeof(struct list_head *));
        if (!group)
            return;
        temp = cur;
        for (int i = 0; i < k; i++) {
            group[i] = temp;
            temp = temp->next;
        }
        struct list_head *before = group[0]->prev;
        struct list_head *after = group[k - 1]->next;
        for (int i = 0; i < k; i++) {
            group[i]->prev = (i == k - 1 ? before : group[i + 1]);
            group[i]->next = (i == 0 ? after : group[i - 1]);
        }
        before->next = group[k - 1];
        after->prev = group[0];
        free(group);
        cur = after;
    }
}

/* =============================================================================
 * q_sort - Sort elements of queue in ascending or descending order.
 *          The implementation copies pointers to an array, uses qsort, and then
 *          rebuilds the linked list.
 * =============================================================================
 */
static int cmp_asc(const void *a, const void *b)
{
    const element_t *elemA = *(const element_t **) a;
    const element_t *elemB = *(const element_t **) b;
    return strcmp(elemA->value, elemB->value);
}

static int cmp_desc(const void *a, const void *b)
{
    const element_t *elemA = *(const element_t **) a;
    const element_t *elemB = *(const element_t **) b;
    return strcmp(elemB->value, elemA->value);
}

void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || head->next->next == head)
        return;
    int n = q_size(head);
    element_t **arr = malloc(n * sizeof(element_t *));
    if (!arr)
        return;
    int i = 0;
    struct list_head *cur;
    list_for_each (cur, head) {
        arr[i++] = container_of(cur, element_t, list);
    }
    if (descend)
        qsort(arr, n, sizeof(element_t *), cmp_desc);
    else
        qsort(arr, n, sizeof(element_t *), cmp_asc);
    INIT_LIST_HEAD(head);
    for (i = 0; i < n; i++) {
        list_add_tail(&arr[i]->list, head);
    }
    free(arr);
}

/* =============================================================================
 * q_ascend - Remove every node which has a node with a strictly less value
 *            anywhere to its right.
 *            (Keep nodes that are not dominated by any smaller node to their
 * right)
 * =============================================================================
 */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head) || head->next->next == head)
        return q_size(head);
    int removed_count = 0;
    struct list_head *cur = head->prev;
    const element_t *last_elem = container_of(cur, element_t, list);
    const char *min_val = last_elem->value;
    cur = cur->prev;
    while (cur != head) {
        element_t *elem = container_of(cur, element_t, list);
        struct list_head *prev_node = cur->prev;
        if (strcmp(elem->value, min_val) > 0) {
            list_del(cur);
            q_release_element(elem);
            removed_count++;
        } else {
            if (strcmp(elem->value, min_val) < 0)
                min_val = elem->value;
        }
        cur = prev_node;
    }
    return q_size(head);
}

/* =============================================================================
 * q_descend - Remove every node which has a node with a strictly greater value
 *             anywhere to its right.
 * =============================================================================
 */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head) || head->next->next == head)
        return q_size(head);
    int removed_count = 0;
    struct list_head *cur = head->prev;
    const element_t *last_elem = container_of(cur, element_t, list);
    const char *max_val = last_elem->value;
    cur = cur->prev;
    while (cur != head) {
        element_t *elem = container_of(cur, element_t, list);
        struct list_head *prev_node = cur->prev;
        if (strcmp(elem->value, max_val) < 0) {
            list_del(cur);
            q_release_element(elem);
            removed_count++;
        } else {
            if (strcmp(elem->value, max_val) > 0)
                max_val = elem->value;
        }
        cur = prev_node;
    }
    return q_size(head);
}

/* =============================================================================
 * merge_two - Helper function: merge sorted list src into dest.
 *             The merge is done in-place; no new allocations.
 * =============================================================================
 */
static void merge_two(struct list_head *dest,
                      struct list_head *src,
                      bool descend)
{
    struct list_head *p = dest->next, *q = src->next;
    while (p != dest && q != src) {
        const element_t *e_p = container_of(p, element_t, list);
        const element_t *e_q = container_of(q, element_t, list);
        int cmp = strcmp(e_p->value, e_q->value);
        if (descend)
            cmp = -cmp;
        if (cmp <= 0) {
            p = p->next;
        } else {
            struct list_head *q_next = q->next;
            list_del(q);
            q->prev = p->prev;
            q->next = p;
            p->prev->next = q;
            p->prev = q;
            q = q_next;
        }
    }
    if (q != src) {
        struct list_head *dest_tail = dest->prev;
        dest_tail->next = q;
        q->prev = dest_tail;
        dest->prev = src->prev;
        src->prev->next = dest;
    }
    INIT_LIST_HEAD(src);
}

/* =============================================================================
 * q_merge - Merge all the queues (from the second onward) in the chain into the
 *           first queue. The queues are assumed to be already sorted.
 *           This function must not allocate any memory.
 * =============================================================================
 */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || head->next == head)
        return 0;
    queue_contex_t *first_ctx = container_of(head->next, queue_contex_t, chain);
    if (!first_ctx->q)
        return 0;
    struct list_head *dest = first_ctx->q;
    struct list_head *cur = head->next->next;
    while (cur != head) {
        queue_contex_t *ctx = container_of(cur, queue_contex_t, chain);
        if (ctx->q) {
            merge_two(dest, ctx->q, descend);
            ctx->q = NULL;
        }
        cur = cur->next;
    }
    return q_size(dest);
}