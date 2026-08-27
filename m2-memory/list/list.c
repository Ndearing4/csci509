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
        // (void)head;
        // (void)value;

        Node *newHead = malloc(sizeof *newHead);
        newHead->next = *head;
        newHead->value = value;

        *head = newHead;
}

int list_pop_front(Node **head, int *out)
{
        // (void)head;
        // (void)out;

        Node *headPointer = *head;
        int *headValue = &headPointer->value; 
        if (headValue == NULL) {
                return 0;
        }

        Node *headCopy = *head;
        *out = (headCopy->value); 
        
        
        *head = ((*head)->next);
        free(headCopy);

        return 1;

}
size_t list_insert_sorted(Node **head, int value)
{
        Node *newNode = malloc(sizeof *newNode);
        newNode->value = value;

        size_t len = list_length(*head);

        Node **walk = head;
        while (*walk != NULL && (*walk)->value < value) {
                walk = &(*walk)->next;
        }

        newNode->next = *walk;
        *walk = newNode;

        return len + 1;
}

int list_remove_first(Node **head, int value)
{
        (void)head;
        (void)value;

        //case 1: empty list
        if (*head == NULL) {
                return 0;
        }
        //case 2: value is at head

        if ((*head)->value == value) {
                list_pop_front(head, &(*head)->value);
                return 1;
        }


        Node **walk;

        for (walk = head; (*walk)->next != NULL; walk = &((*walk)->next)){
                //case 3: middle of list
                if ((*walk)->value == value) {
                        list_pop_front(walk, &(*walk)->value);
                        return 1;
                }
        }

        //case 4: End of list (maybe not necessary?)
        if ((*walk)->value == value) {
                free(*walk);
                *walk = NULL;
                return 1; 
        }


        return 0;
}

size_t list_length(const Node *head)
{
        // (void)head;

        size_t count = 0;

        const Node *walk;


        for (walk = &*head; walk != NULL; walk = walk->next) {
                count = count + 1;
        }

        return count;
}

int list_contains(const Node *head, int value)
{
        // (void)head;
        // (void)value;

        const Node *walk;

        for (walk = &*head; walk != NULL; walk = walk->next) {
                if (walk->value == value) {
                        return 1;
                }
        }

        return 0;
}

size_t list_to_array(const Node *head, int *out, size_t max)
{
        // (void)head;
        // (void)out;
        // (void)max;

        // out = malloc(max * sizeof *out);

        const Node *walk = head;

        size_t outLen = 0;

        for (size_t i = 0; i < max; i++) {
                if (walk!= NULL) {

                        out[i] = walk->value;
                        outLen += 1;
                        walk = walk->next;
                }
        }

        return outLen;
}

void list_reverse(Node **head)
{
        // (void)head;

        struct Node *curr = *head, *prev = NULL, *next;

        while (curr != NULL) {
                next = curr->next;
                curr->next = prev;
                prev = curr;
                curr = next;
        }

        *head = prev;
}

void list_free(Node **head)
{
        // (void)head;

        Node *curr = *head, *prev = NULL;

        // free(head);

        while (curr != NULL) {
                prev = curr;
                curr = curr->next;
                free(prev);
        }

        *head = NULL;

        
}
