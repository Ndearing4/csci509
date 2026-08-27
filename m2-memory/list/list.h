/*
 * list.h -- M2 exit test: a singly linked list of ints.
 * Nathan Dearing, 2026-08-21.
 *
 * The header is the interface. A caller who has read this file and never
 * opened list.c should be able to use the list correctly -- that is the test
 * of whether the comments here are doing their job.
 *
 * Ownership rule for the whole file: the list owns every node in it. Nothing
 * here hands you a node to free yourself. list_free is the only way nodes go
 * away, and after it the caller's head pointer is NULL.
 */

#ifndef LIST_H
#define LIST_H
/* Header guard -- prevents this file's Node typedef and prototypes from
 * being parsed twice if two .c files that both need list.h land in the
 * same build. Full explanation: c-structs/counters.h. */

#include <stddef.h>

/* One link. `next` is NULL at the end of the list; an empty list is a NULL
 * head pointer, not a node holding nothing. */
typedef struct Node {
        int value;              /* the payload */
        struct Node *next;      /* the next link, or NULL */
} Node;

/*
 * WHY Node ** AND NOT Node *
 *
 * Five of the nine functions below take `Node **head`. Before implementing
 * any of them, write this one with a single `Node *head` parameter:
 *
 *      void list_push_front(Node *head, int value)
 *      {
 *              Node *n = malloc(sizeof *n);
 *              n->value = value;
 *              n->next = head;
 *              head = n;               <-- assigns to the parameter
 *      }
 *
 * Compile it, call it, then print the caller's head. It is unchanged. C
 * passes everything by value, and a `Node *` parameter is a *copy* of the
 * caller's pointer; assigning to the copy cannot reach the original. To
 * modify the caller's pointer you need its address, which is a `Node **`.
 *
 * Do that experiment for real. It is the M2 skip-ahead probe and the reason
 * this exercise exists.
 */

/* Insert `value` at the front. O(1). Aborts on allocation failure -- this is
 * a teaching list, not a library, and error paths would obscure the shape.
 * The caller's head pointer is updated. */
void list_push_front(Node **head, int value);

/* Remove the first node and store its value through `out`. Returns 1 if a
 * node was removed, 0 if the list was empty (in which case *out is not
 * touched). Frees the removed node. `out` may not be NULL. */
int list_pop_front(Node **head, int *out);

/* Insert `value` so the list stays in non-decreasing order, assuming it
 * already was. Equal values may go on either side of each other. Returns the
 * new length.
 *
 * This is the one where the double pointer earns its keep: inserting before
 * the first element changes the caller's head, inserting anywhere else does
 * not, and you should be able to write it *without* special-casing the front.
 * Hint: walk a `Node **` down the list instead of a `Node *`. */
size_t list_insert_sorted(Node **head, int value);

/* Remove the first node whose value is `value`. Returns 1 if one was removed,
 * 0 if no node matched. Frees the removed node.
 *
 * The three cases that get broken: the match is the first node, the match is
 * the last node, the list is empty. The harness checks all three. */
int list_remove_first(Node **head, int value);

/* Number of nodes. 0 for an empty list. Does not modify anything, hence the
 * `const Node *` -- and hence no double pointer. */
size_t list_length(const Node *head);

/* 1 if any node holds `value`, else 0. */
int list_contains(const Node *head, int value);

/* Copy up to `max` values into `out`, front to back. Returns how many were
 * written, which is min(list_length, max). Writes nothing if `max` is 0.
 * `out` must have room for `max` ints. */
size_t list_to_array(const Node *head, int *out, size_t max);

/* Reverse the list in place, without allocating and without moving any
 * values between nodes -- relink the nodes themselves. The caller's head ends
 * up pointing at what used to be the last node.
 *
 * Three pointers, one pass. If you find yourself wanting a fourth, the third
 * one is doing two jobs. */
void list_reverse(Node **head);

/* Free every node and set the caller's head to NULL. Safe on an empty list.
 * Safe to call twice, which is exactly why it takes a double pointer: after
 * it runs the caller cannot be holding a dangling pointer, because it is
 * holding NULL.
 *
 * The bug to avoid: freeing a node and then reading its `next`. That is a
 * use-after-free, it will appear to work, and it is bugs/bug1. */
void list_free(Node **head);

#endif /* LIST_H */
