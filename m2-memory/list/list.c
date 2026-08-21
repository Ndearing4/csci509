/*
 * list.c -- M2 exit test: a singly linked list of ints.
 * Nathan Dearing, 2026-08-21.
 *
 * Nine stubs. The contracts are in list.h -- read them there, not here, and
 * keep it that way when you implement: a comment describing *what* a function
 * does belongs in the header where callers see it, and a comment describing
 * *how* belongs down here.
 *
 * Every stub currently ignores its arguments via (void) casts so the file
 * builds clean under -Wall -Wextra -Werror before you start. Delete each cast
 * as you implement that function.
 *
 * Order to work in: push_front, length, to_array, free -- that gets the
 * harness's first groups running and gives you something to look at in gdb.
 * Then pop_front, contains, remove_first. Then insert_sorted and reverse,
 * which are the two that are actually interesting.
 *
 * Anything you malloc here gets freed by list_free or by the function that
 * removes the node. Check with:
 *     valgrind --leak-check=full --track-origins=yes ./test_list
 */

#include <stdlib.h>
#include "list.h"

void list_push_front(Node **head, int value)
{
        (void)head;
        (void)value;
}

int list_pop_front(Node **head, int *out)
{
        (void)head;
        (void)out;
        return 0;
}

size_t list_insert_sorted(Node **head, int value)
{
        (void)head;
        (void)value;
        return 0;
}

int list_remove_first(Node **head, int value)
{
        (void)head;
        (void)value;
        return 0;
}

size_t list_length(const Node *head)
{
        (void)head;
        return 0;
}

int list_contains(const Node *head, int value)
{
        (void)head;
        (void)value;
        return 0;
}

size_t list_to_array(const Node *head, int *out, size_t max)
{
        (void)head;
        (void)out;
        (void)max;
        return 0;
}

void list_reverse(Node **head)
{
        (void)head;
}

void list_free(Node **head)
{
        (void)head;
}
