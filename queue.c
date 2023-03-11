#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (!new) {
        return NULL;
    }

    INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l) {
        return;
    }

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list) {
        list_del(&entry->list);  // can call q_release_element() directly, but
                                 // including list_del would be fine
        q_release_element(entry);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = (element_t *) malloc(sizeof(element_t));
    if (!new)
        return false;

    int len = strlen(s);
    char *buf = (char *) malloc((len + 1) * sizeof(char));
    if (!buf) {
        free(new);
        return false;
    }

    strncpy(buf, s, len);
    *(buf + len) = 0;
    new->value = buf;
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = (element_t *) malloc(sizeof(element_t));
    if (!new)
        return false;

    int len = strlen(s);
    char *buf = (char *) malloc((len + 1) * sizeof(char));
    if (!buf) {
        free(new);
        return false;
    }

    strncpy(buf, s, len);
    *(buf + len) = 0;
    new->value = buf;
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_first_entry(head, element_t, list);
    list_del_init(&entry->list);

    if (sp) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_last_entry(head, element_t, list);
    list_del_init(&entry->list);

    if (sp) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *fast = head->next, *slow = head->next;
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    // remove node from list
    list_del(slow);

    // free buffer of element
    q_release_element(list_entry(slow, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    element_t *curr, *next;
    bool flag = false;
    list_for_each_entry_safe (curr, next, head, list) {
        if (curr->list.next != head && strcmp(curr->value, next->value) == 0) {
            list_del(&curr->list);
            q_release_element(curr);
            flag = true;
        } else if (flag) {
            list_del(&curr->list);
            q_release_element(curr);
            flag = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    for (struct list_head *c = head->next; c != head && c->next != head;
         c = c->next) {
        list_move(c, c->next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    // struct list_head *first = head->next, *safe;
    // for (struct list_head *curr = head->next; curr != first; curr = safe) {
    //     safe = curr->next;
    //     list_move_tail(curr, head);
    // }
    struct list_head *c, *n;
    list_for_each_safe (c, n, head) {
        list_move(c, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || k < 0)
        return;
    struct list_head *c, *n, k_list, *tmp_head = head;
    INIT_LIST_HEAD(&k_list);
    int i = 0;
    list_for_each_safe (c, n, head) {
        i++;
        if (i == k) {
            list_cut_position(&k_list, tmp_head, c);
            q_reverse(&k_list);
            list_splice_init(&k_list, tmp_head);
            i = 0;
            tmp_head = n->prev;
        }
    }
}

/* Utility functions for q_sort, implementing mergesort */
struct list_head *merge_two_list(struct list_head *l1, struct list_head *l2)
{
    if (!l1 && !l2)
        return NULL;
    else if (!l1)
        return l2;
    else if (!l2)
        return l1;

    struct list_head *head, *ptr;
    if (strcmp(list_entry(l1, element_t, list)->value,
               list_entry(l2, element_t, list)->value) < 0) {
        head = l1;
        l1 = l1->next;
    } else {
        head = l2;
        l2 = l2->next;
    }
    ptr = head;

    while (l1 && l2) {
        if (strcmp(list_entry(l1, element_t, list)->value,
                   list_entry(l2, element_t, list)->value) < 0) {
            ptr->next = l1;
            l1->prev = ptr;
            l1 = l1->next;
        } else {
            ptr->next = l2;
            l2->prev = ptr;
            l2 = l2->next;
        }
        ptr = ptr->next;
    }

    if (l1) {
        ptr->next = l1;
        l1->prev = ptr;
    }
    if (l2) {
        ptr->next = l2;
        l2->prev = ptr;
    }

    while (ptr->next)
        ptr = ptr->next;
    head->prev = ptr;
    return head;
}

struct list_head *mergeSort(struct list_head *head)
{
    if (!head->next)
        return head;
    struct list_head *fast = head, *slow = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    slow->prev->next = NULL;
    return merge_two_list(mergeSort(head), mergeSort(slow));
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    head->prev->next = NULL;
    head->next = mergeSort(head->next);
    head->next->prev->next = head;
    head->prev = head->next->prev;
    head->next->prev = head;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int len = 0;
    element_t *last = list_entry(head->prev, element_t, list);
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    for (element_t *curr = list_entry(head->prev, element_t, list),
                   *safe = list_entry(curr->list.prev, element_t, list);
         &curr->list != (head);
         curr = safe, safe = list_entry(safe->list.prev, element_t, list)) {
        if (strcmp(curr->value, last->value) < 0) {
            list_del(&curr->list);
            q_release_element(curr);
        } else {
            last = curr;
            len++;
        }
    }
    return len;
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    // if (!head)
    //     return 0;
    // if (list_is_singular(head))
    //     return list_entry(head, queue_contex_t, chain)->size;
    // // https://leetcode.com/problems/merge-k-sorted-lists/


    // queue_contex_t *first = list_entry(head->next, queue_contex_t, chain);
    // if (!list_empty(first->q))
    //     first->q->prev->next = NULL;
    // fprintf(stderr, "%d\n", q_size(first->q));
    // int len = first->size;
    // for (struct list_head *cur = head->next->next; cur != head;
    //      cur = cur->next) {
    //     queue_contex_t *curr = list_entry(cur, queue_contex_t, chain);
    //     if (list_empty(curr->q)) {
    //         continue;
    //     }
    //     fprintf(stderr, "%p %p\n", first->q, first->q->next);
    //     curr->q->prev->next = NULL;  // break the circular structure for
    //     merging first->q->next = merge_two_list(first->q->next,
    //     curr->q->next); len += curr->size; curr->q = NULL; curr->size = 0;
    // }
    // // now first->q is a linked-list (not circular), so we have to reconnect
    // it. struct list_head *tail = first->q->next; while (tail->next) {
    //     tail = tail->next;
    // }

    // tail->next = first->q;
    // first->q->prev = tail;
    // first->size = len;

    // // queue_contex_t *q_head = list_first_entry(head, queue_contex_t,
    // chain); return len;

    if (!head || list_empty(head))
        return 0;

    queue_contex_t *q1 = container_of(head->next, queue_contex_t, chain);
    if (list_is_singular(head))
        return q1->size;
    for (struct list_head *cur = head->next->next; cur != head;
         cur = cur->next) {
        queue_contex_t *q = container_of(cur, queue_contex_t, chain);
        list_splice_init(q->q, q1->q);
        q->size = 0;
    }
    q_sort(q1->q);
    q1->size = q_size(q1->q);

    return q1->size;
}
