/*
 * parse.c -- turn a line of input into commands the shell can run.
 * Nathan Dearing, 2026-08-21.
 *
 * Contracts are in parse.h.
 *
 * Write this one first and finish it before touching shell.c. It is pure
 * string handling -- no syscalls, nothing to debug in two processes at once
 * -- and every stage after this assumes it is correct. A parser bug found
 * while you are also debugging a pipe is two bugs that look like one.
 *
 * Suggested approach, and the reason strtok is not in it:
 *
 *   strtok(line, " \t\n") gets you words in about four lines, and then you
 *   discover it cannot tell `ls>out` from `ls > out`, keeps state in a
 *   static variable so you cannot nest it, and modifies its input in a way
 *   that fights the second pass. Walk the string with a pointer instead:
 *   skip spaces, note the start, walk to the next space or special
 *   character, write a '\0', record the pointer. Twenty lines, no surprises.
 *
 *   Treat < > | as complete words even with no space around them, so that
 *   `ls>out.txt` parses the same as `ls > out.txt`. That is one extra check
 *   in the word loop and it removes a whole class of confusing failures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

/*
 * TODO 1 -- parse_line
 *
 * See parse.h for the contract and the list of syntax errors to catch.
 *
 * Shape that works:
 *
 *     zero the Pipeline
 *     for each word in line:
 *         "|"  -> finish the current Command, start the next one
 *         "<"  -> the NEXT word is infile (error if there is no next word)
 *         ">"  -> the next word is outfile, append = 0
 *         ">>" -> the next word is outfile, append = 1
 *         else -> argv[argc++] = word
 *     NULL-terminate each argv
 *
 * Do not forget the NULL terminator on argv. execvp reads until it finds
 * one, and if it is not there it reads whatever was next in the struct.
 * Zeroing the whole Pipeline at the top makes this correct by default, which
 * is a good enough reason to do it that way.
 */
int parse_line(char *line, Pipeline *pl)
{
        (void)line;
        (void)pl;
        return -1;
}

/*
 * TODO 2 -- parse_dump
 *
 * One line per command, showing argc, every argv entry, and any redirections.
 * Something like:
 *
 *     [0] argc=2  argv={"ls","-la"}  >out.txt
 *     [1] argc=2  argv={"grep","foo"}
 *
 * Worth ten minutes. It is the difference between "the pipe does not work"
 * and "the second command has argc=0", and you will read it a hundred times
 * during stage 3.
 */
void parse_dump(const Pipeline *pl)
{
        (void)pl;
        printf("(parse_dump not implemented)\n");
}
