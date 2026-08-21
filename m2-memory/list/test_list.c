/*
 * test_list.c -- harness for the M2 linked list.
 * Nathan Dearing, 2026-08-21.
 *
 * Nine groups, one per function, run in the order list.c suggests you
 * implement them. Everything the harness checks it checks by walking the
 * nodes itself -- it never uses one of your functions to verify another, so a
 * broken list_length cannot make list_push_front look correct.
 *
 * With the stubs in place every group fails. That is the starting state.
 *
 * The harness leaks nothing of its own: every list it builds is torn down
 * with list_free, so once the list is correct this is valgrind-clean. If
 * valgrind reports a leak, it is yours.
 */

#include <stdio.h>
#include <stdlib.h>
#include "list.h"

static int checks_run;          /* total assertions attempted */
static int checks_failed;       /* how many of those failed */
static int group_failed;        /* failures in the current group */

static void group(const char *name)
{
        if (checks_run > 0)
                printf("%s\n", group_failed ? "  ^ group FAILED" : "  ok");
        group_failed = 0;
        printf("--- %s\n", name);
}

/* Record one assertion. `detail` is printed only on failure. */
static void check(int ok, const char *what, const char *detail)
{
        checks_run++;
        if (!ok) {
                checks_failed++;
                group_failed++;
                printf("  FAIL  %s\n", what);
                if (detail != NULL)
                        printf("        %s\n", detail);
        }
}

/* Walk the list by hand and compare against `want`. Deliberately does not
 * call list_length or list_to_array -- the harness must not depend on the
 * code it is testing. */
static void check_shape(const Node *head, const int *want, size_t n,
                        const char *what)
{
        const Node *p = head;
        size_t i = 0;
        char detail[256];
        int off = 0;

        while (p != NULL && i < n) {
                if (p->value != want[i]) {
                        snprintf(detail, sizeof detail,
                                 "at position %zu: got %d, want %d",
                                 i, p->value, want[i]);
                        check(0, what, detail);
                        return;
                }
                p = p->next;
                i++;
        }

        if (p != NULL) {
                off = snprintf(detail, sizeof detail,
                               "longer than the expected %zu, extra:", n);
                while (p != NULL && off < (int)sizeof detail - 16) {
                        off += snprintf(detail + off, sizeof detail - (size_t)off,
                                        " %d", p->value);
                        p = p->next;
                }
                check(0, what, detail);
                return;
        }
        if (i != n) {
                snprintf(detail, sizeof detail,
                         "ended after %zu values, expected %zu", i, n);
                check(0, what, detail);
                return;
        }
        check(1, what, NULL);
}

/* Build a list front-to-back holding exactly `want`, using push_front only.
 * If push_front is broken every group breaks, which is why it is group 1. */
static void build(Node **head, const int *want, size_t n)
{
        size_t i = n;

        *head = NULL;
        while (i > 0) {
                i--;
                list_push_front(head, want[i]);
        }
}

int main(void)
{
        printf("=== M2 linked list ===\n\n");

        group("1. push_front and length");
        {
                Node *head = NULL;
                int want3[] = { 30, 20, 10 };

                check(list_length(head) == 0, "length of an empty list is 0", NULL);

                list_push_front(&head, 10);
                check(head != NULL, "push_front updated the caller's head pointer",
                      "if head is still NULL you took Node * instead of Node **");
                check(list_length(head) == 1, "length after one push is 1", NULL);

                list_push_front(&head, 20);
                list_push_front(&head, 30);
                check_shape(head, want3, 3, "three pushes come out 30, 20, 10");
                check(list_length(head) == 3, "length after three pushes is 3", NULL);

                list_free(&head);
        }

        group("2. to_array");
        {
                Node *head = NULL;
                int want[] = { 1, 2, 3, 4, 5 };
                int got[8];
                size_t n;

                build(&head, want, 5);

                for (n = 0; n < 8; n++)
                        got[n] = -1;
                n = list_to_array(head, got, 8);
                check(n == 5, "returns the count written when max exceeds length", NULL);
                check(got[0] == 1 && got[4] == 5, "values come out front to back", NULL);
                check(got[5] == -1, "does not write past what it returned", NULL);

                n = list_to_array(head, got, 3);
                check(n == 3, "returns max when the list is longer", NULL);
                check(got[2] == 3, "wrote the first three values", NULL);

                n = list_to_array(head, got, 0);
                check(n == 0, "max of 0 writes nothing and returns 0", NULL);

                list_free(&head);
        }

        group("3. pop_front");
        {
                Node *head = NULL;
                int src[] = { 7, 8 };
                int rest[] = { 8 };
                int out = -1;

                build(&head, src, 2);

                check(list_pop_front(&head, &out) == 1, "returns 1 when it popped", NULL);
                check(out == 7, "stored the front value through out", NULL);
                check_shape(head, rest, 1, "the popped node is gone");

                out = -1;
                check(list_pop_front(&head, &out) == 1, "pops the last node", NULL);
                check(out == 8, "stored the last value", NULL);
                check(head == NULL, "head is NULL after popping the last node",
                      "the caller must not be left pointing at a freed node");

                out = -1;
                check(list_pop_front(&head, &out) == 0, "returns 0 on an empty list", NULL);
                check(out == -1, "leaves *out alone when there was nothing to pop", NULL);
        }

        group("4. contains");
        {
                Node *head = NULL;
                int src[] = { 4, 5, 6 };

                check(list_contains(NULL, 4) == 0, "empty list contains nothing", NULL);

                build(&head, src, 3);
                check(list_contains(head, 4) == 1, "finds the first value", NULL);
                check(list_contains(head, 5) == 1, "finds a middle value", NULL);
                check(list_contains(head, 6) == 1, "finds the last value", NULL);
                check(list_contains(head, 99) == 0, "returns 0 for a value not present", NULL);

                list_free(&head);
        }

        group("5. remove_first");
        {
                Node *head = NULL;
                int src[] = { 1, 2, 3 };
                int no_first[] = { 2, 3 };
                int no_mid[] = { 1, 3 };
                int no_last[] = { 1, 2 };

                check(list_remove_first(&head, 1) == 0, "returns 0 on an empty list", NULL);

                build(&head, src, 3);
                check(list_remove_first(&head, 1) == 1, "returns 1 when it removed", NULL);
                check_shape(head, no_first, 2, "removing the head updates the caller's pointer");
                list_free(&head);

                build(&head, src, 3);
                list_remove_first(&head, 2);
                check_shape(head, no_mid, 2, "removing from the middle relinks");
                list_free(&head);

                build(&head, src, 3);
                list_remove_first(&head, 3);
                check_shape(head, no_last, 2, "removing the tail terminates the list");
                list_free(&head);

                build(&head, src, 3);
                check(list_remove_first(&head, 99) == 0, "returns 0 when nothing matches", NULL);
                check_shape(head, src, 3, "a failed remove leaves the list alone");
                list_free(&head);
        }

        group("6. insert_sorted");
        {
                Node *head = NULL;
                int want1[] = { 5 };
                int want2[] = { 2, 5 };
                int want3[] = { 2, 5, 9 };
                int want4[] = { 2, 4, 5, 9 };
                int want5[] = { 2, 4, 5, 5, 9 };

                check(list_insert_sorted(&head, 5) == 1, "returns the new length", NULL);
                check_shape(head, want1, 1, "insert into an empty list");

                list_insert_sorted(&head, 2);
                check_shape(head, want2, 2, "insert before the head");

                list_insert_sorted(&head, 9);
                check_shape(head, want3, 3, "insert at the end");

                list_insert_sorted(&head, 4);
                check_shape(head, want4, 4, "insert into the middle");

                check(list_insert_sorted(&head, 5) == 5, "returns 5 for the fifth insert", NULL);
                check_shape(head, want5, 5, "a duplicate lands next to its twin");

                list_free(&head);
        }

        group("7. reverse");
        {
                Node *head = NULL;
                int one[] = { 42 };
                int two[] = { 1, 2 };
                int two_r[] = { 2, 1 };
                int many[] = { 1, 2, 3, 4, 5 };
                int many_r[] = { 5, 4, 3, 2, 1 };

                list_reverse(&head);
                check(head == NULL, "reversing an empty list leaves it empty", NULL);

                build(&head, one, 1);
                list_reverse(&head);
                check_shape(head, one, 1, "reversing one node changes nothing");
                list_free(&head);

                build(&head, two, 2);
                list_reverse(&head);
                check_shape(head, two_r, 2, "reversing two nodes swaps them");
                list_free(&head);

                build(&head, many, 5);
                list_reverse(&head);
                check_shape(head, many_r, 5, "reversing five nodes");
                list_reverse(&head);
                check_shape(head, many, 5, "reversing twice is the identity");
                list_free(&head);
        }

        group("8. free");
        {
                Node *head = NULL;
                int src[] = { 1, 2, 3 };

                list_free(&head);
                check(head == NULL, "freeing an empty list is safe", NULL);

                build(&head, src, 3);
                list_free(&head);
                check(head == NULL, "free sets the caller's head to NULL",
                      "this is what makes the second free below safe");

                list_free(&head);
                check(head == NULL, "freeing an already-freed list is safe", NULL);
        }

        group("9. a thousand nodes, for valgrind");
        {
                Node *head = NULL;
                int i;
                int out = 0;

                for (i = 0; i < 1000; i++)
                        list_push_front(&head, i);
                check(list_length(head) == 1000, "1000 nodes went in", NULL);

                list_reverse(&head);
                check(list_contains(head, 999) == 1, "still holds 999 after reverse", NULL);

                for (i = 0; i < 500; i++)
                        list_pop_front(&head, &out);
                check(list_length(head) == 500, "500 left after 500 pops", NULL);

                list_free(&head);
                check(head == NULL, "all 1000 accounted for", NULL);
        }

        printf("%s\n", group_failed ? "  ^ group FAILED" : "  ok");

        printf("\n=== %d checks, %d failed ===\n", checks_run, checks_failed);
        if (checks_failed == 0) {
                printf("All groups pass. Now run it under valgrind:\n");
                printf("    valgrind --leak-check=full --track-origins=yes ./test_list\n");
                printf("Passing tests and a clean valgrind are two separate bars.\n");
                return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
}
