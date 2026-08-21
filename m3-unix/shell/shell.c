/*
 * shell.c -- the M3 project: a small shell.
 * Nathan Dearing, 2026-08-21.
 *
 * Read STAGES.md before writing anything here. There are four stages and
 * they are ordered so that each one is debuggable on its own; doing them out
 * of order means debugging a pipe and a signal handler at the same time,
 * which is how this project turns into a week.
 *
 * Exit test, from the top-level README:
 *     ls -la | grep foo > out.txt    works
 *     Ctrl-C kills the child, not the shell
 *     ps shows no zombies after a hundred commands
 *
 * ./test-shell.sh checks everything except the Ctrl-C behaviour, which needs
 * a real terminal -- STAGES.md stage 4 has the manual checklist for that.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parse.h"

#define LINE_MAX_LEN 4096

/* Set by the SIGINT handler. `volatile sig_atomic_t` is the only type you may
 * touch from a handler and read from the main flow -- see STAGES.md stage 4
 * for why, and for the list of what else is safe in there (it is short). */
static volatile sig_atomic_t interrupted;

/*
 * STAGE 4 -- on_sigint
 *
 * Runs on Ctrl-C. Do essentially nothing in here: set the flag, and at most
 * write(2) a newline so the prompt lands on a fresh line.
 *
 * printf is NOT safe here. If the interrupt lands while the main flow is
 * halfway through printf's own buffer bookkeeping, calling printf again
 * re-enters it on inconsistent state. `man 7 signal-safety` lists the calls
 * that are safe; write is on the list, printf is not.
 */
static void on_sigint(int signo)
{
        (void)signo;
        interrupted = 1;
}

/*
 * STAGE 2 -- apply_redirections
 *
 * In the CHILD, after fork and before exec, point stdin and stdout at the
 * files the command asked for. Return 0 on success, -1 after printing why.
 *
 *     <  file    open(file, O_RDONLY)
 *     >  file    open(file, O_WRONLY | O_CREAT | O_TRUNC,  0644)
 *     >> file    open(file, O_WRONLY | O_CREAT | O_APPEND, 0644)
 *
 * then dup2(fd, STDIN_FILENO) or dup2(fd, STDOUT_FILENO), then close(fd).
 *
 * What dup2(old, new) does: makes `new` refer to the same open file as
 * `old`, closing whatever `new` was first. It does not copy any data. After
 * it, the process has two descriptors onto one open file description, which
 * is why you then close the original -- the file stays open through the
 * other one, and leaving spare copies around is what breaks pipes in stage 3.
 *
 * This runs in the child ONLY. Do it in the parent and you have redirected
 * the shell's own stdout into out.txt, permanently, and the next prompt
 * disappears into the file. Try that once deliberately; it is instructive.
 *
 * On failure the child must _exit(1), not return -- it is a copy of the
 * shell, and letting it fall back into the main loop gives you two shells
 * reading the same terminal.
 */
static int apply_redirections(const Command *cmd)
{
        (void)cmd;
        return -1;
}

/*
 * STAGE 1 -- run_simple
 * STAGE 3 -- run_pipeline
 *
 * Stage 1: fork, apply redirections in the child, execvp, waitpid in the
 * parent. This is procs/spawn.c with two extra lines. Return the exit status.
 *
 * Stage 3: n commands joined by pipes. For each adjacent pair, pipe(fds)
 * gives you fds[0] to read and fds[1] to write; the left command's stdout is
 * dup2'd onto fds[1] and the right command's stdin onto fds[0].
 *
 * The bug you will have, in advance: it hangs.
 *
 * A read on a pipe returns EOF when NO process still has the write end open,
 * and the shell forked before it closed anything, so the shell still has it.
 * grep waits for an EOF the kernel will not send because the shell is still
 * holding a write end it is never going to write to. Every process must close
 * every pipe descriptor it is not personally using -- including the parent,
 * and including the ends belonging to other commands in the same pipeline.
 *
 * Count them out on paper for `a | b | c` before writing the loop: two pipes,
 * four descriptors, four processes counting the shell, and write down which
 * of the sixteen combinations each process keeps. That table is the exercise.
 *
 * Fork ALL the children first, then wait for all of them. Waiting for the
 * first before starting the second deadlocks as soon as the first command
 * writes more than a pipe buffer -- 64 KB on Linux -- because nothing is
 * draining the other end.
 *
 * The pipeline's exit status is the LAST command's, which is what shells do
 * and what test-shell.sh checks. Reap the others anyway.
 */
static int run_pipeline(const Pipeline *pl)
{
        (void)pl;
        return -1;
}

/*
 * STAGE 1 -- builtins
 *
 * `exit` and `cd` cannot be external programs, and working out why is the
 * point of this function.
 *
 * cd in a child process changes the child's working directory and then the
 * child exits. The shell's directory is untouched. A builtin is not an
 * optimisation here -- it is the only thing that can work, because the state
 * being changed belongs to the shell process itself.
 *
 * Return 1 if it handled the command, 0 if it was not a builtin.
 * `cd` with no argument goes to $HOME. Check chdir's return value; a `cd` to
 * a nonexistent directory should print an error, not silently do nothing.
 */
static int run_builtin(const Command *cmd, int *shell_should_exit)
{
        (void)cmd;
        (void)shell_should_exit;
        return 0;
}

/*
 * STAGE 1 -- main
 *
 * The loop: print a prompt, read a line, parse it, run it, repeat until EOF.
 *
 *   - Prompt to stdout, then fflush(stdout). Without the flush the prompt
 *     sits in the buffer and the shell looks hung -- syscalls/buffering.c
 *     section 1, met again in the wild.
 *   - Print the prompt only when stdin is a terminal (isatty). test-shell.sh
 *     pipes commands in, and a prompt in the middle of the output makes
 *     every comparison fail.
 *   - fgets returning NULL means EOF (Ctrl-D) -- exit cleanly.
 *   - A blank line is not an error. Parse it, get 0 commands, prompt again.
 *   - The shell's own exit status is the last command's, like every shell.
 */
int main(void)
{
        char line[LINE_MAX_LEN];
        Pipeline pl;
        int last_status = 0;
        int should_exit = 0;

        (void)pl;
        (void)line;
        (void)should_exit;
        (void)on_sigint;
        (void)apply_redirections;
        (void)run_pipeline;
        (void)run_builtin;
        (void)interrupted;

        fprintf(stderr, "shell: not implemented yet -- see STAGES.md\n");
        return last_status == 0 ? 1 : last_status;
}
