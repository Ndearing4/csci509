/*
 * parse.h -- turn a line of input into commands the shell can run.
 * Nathan Dearing, 2026-08-21.
 *
 * Deliberately small. This is not a shell grammar: no quoting, no variables,
 * no globbing, no && or ;. Those are all worth knowing about and none of them
 * teach anything about the kernel, which is what M3 is for. What is here is
 * exactly what the exit test needs:
 *
 *     ls -la | grep foo > out.txt
 *
 * Whitespace-separated words, optional < and > and >> on any command,
 * optional | between commands.
 */

#ifndef PARSE_H
#define PARSE_H

#define MAX_ARGS  64            /* words in one command, not counting NULL */
#define MAX_CMDS  8             /* commands in one pipeline */

/* One command in a pipeline, with its own redirections.
 *
 * Every char * in here points INTO the caller's line buffer -- the parser
 * writes '\0' over the separators rather than copying anything out. So the
 * line buffer has to stay alive and unmodified for as long as the Command
 * is in use, and nothing in here is ever freed. That is how real shells do
 * it too, and it is worth understanding before you decide it is a shortcut. */
typedef struct Command {
        char *argv[MAX_ARGS + 1];  /* NULL-terminated, ready for execvp */
        int argc;                  /* words in argv, not counting the NULL */
        char *infile;              /* file after <, or NULL */
        char *outfile;             /* file after > or >>, or NULL */
        int append;                /* 1 if it was >>, 0 if > */
} Command;

/* A whole line: one or more commands joined by |. */
typedef struct Pipeline {
        Command cmds[MAX_CMDS];
        int ncmds;                 /* 0 for a blank line */
} Pipeline;

/* Parse `line` into `pl`.
 *
 * Returns the number of commands (0 for a blank or comment-only line), or -1
 * on a syntax error, having already printed a message to stderr.
 *
 * MODIFIES `line` IN PLACE. Do not pass it a string literal, do not reuse the
 * buffer while `pl` is live, and do not free it before the pipeline has run.
 *
 * Syntax errors it must catch, because the tests feed them to you:
 *     ls |            a pipe with nothing after it
 *     | ls            a pipe with nothing before it
 *     ls >            a redirect with no filename
 *     > out.txt       a redirect with no command
 *     too many words / too many commands
 * "Catch" means print something to stderr and return -1. It does NOT mean
 * crash, and it does not mean run part of the line. */
int parse_line(char *line, Pipeline *pl);

/* Print a parsed pipeline, one line per command, for debugging.
 * Stage 1 will not work without this and stage 3 is unreadable without it. */
void parse_dump(const Pipeline *pl);

#endif /* PARSE_H */
