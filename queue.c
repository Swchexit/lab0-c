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
        if (entry->value) {  // I think this is imperative, but no one mentioned
                             // it.
            free(entry->value);
        }
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

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
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
    // https://leetcode.com/problems/merge-k-sorted-lists/

    return 0;
}
