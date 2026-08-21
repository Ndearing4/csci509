/*
 * spawn.c -- run one command in a child process and report how it ended.
 * Nathan Dearing, 2026-08-21.
 *
 * The whole of fork/exec/wait in one small program, and the direct
 * prerequisite for shell/. Get this right and stage 1 of the shell is
 * twenty lines.
 *
 *     ./spawn ls -la
 *     ./spawn sh -c 'exit 42'
 *     ./spawn sleep 5          (then Ctrl-C it, and see what it reports)
 *
 * The thing worth understanding before writing any of it: fork and exec are
 * two operations, not one, and every other design puts them together.
 * fork gives you a second copy of *this* program; exec replaces the program
 * running in a process without creating a new one. The gap between them --
 * where you are a copy of the parent but have not become the new program yet
 * -- is where the child sets up its own file descriptors, and it is the
 * reason UNIX redirection and pipes are a shell feature rather than a kernel
 * feature. Every `>` and `|` in stage 2 and 3 happens in that gap.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * TODO 1 -- describe_status
 *
 * Turn a status word from wait/waitpid into a shell-style exit code, and
 * print one human-readable line about it to stdout.
 *
 * A status is NOT an exit code. It is a packed word, and you take it apart
 * with the macros in <sys/wait.h>:
 *
 *     WIFEXITED(st)     ended by calling exit() or returning from main
 *       WEXITSTATUS(st)   ... and this was the value, 0-255
 *     WIFSIGNALED(st)   killed by a signal
 *       WTERMSIG(st)      ... and this was the signal number
 *     WIFSTOPPED(st)    stopped, not dead (only if you passed WUNTRACED)
 *
 * Never print `status` itself and never compare it to 0. `exit(1)` and
 * "killed by SIGHUP" are different events and the packed word says so;
 * treating the word as a number loses that and is wrong in a way that looks
 * right for exit code 0.
 *
 * Return, matching what every shell does so the tests can check it:
 *     normal exit    -> WEXITSTATUS(st)
 *     killed by sig  -> 128 + WTERMSIG(st)
 *
 * strsignal(sig) gives the signal's name for the message.
 */
static int describe_status(int status)
{
        (void)status;
        return -1;
}

/*
 * TODO 2 -- spawn
 *
 * Run argv[0] with arguments argv in a child process, wait for it, and
 * return what describe_status returns.
 *
 * The shape:
 *
 *     pid = fork();
 *     if (pid == -1)   -> fork failed, report and return -1
 *     if (pid == 0)    -> child:  execvp(argv[0], argv);
 *                                 if it returns, it FAILED
 *     else             -> parent: waitpid(pid, &status, 0)
 *
 * Five things to get right, and the tests check all five:
 *
 *   - Test for -1 BEFORE testing for 0. A program that treats -1 as "I am the
 *     child" runs the child path in the parent.
 *
 *   - exec only returns on failure. There is no "if it worked" branch --
 *     if execvp returns at all, it returned -1 and errno says why. The line
 *     after execvp is an error path, always.
 *
 *   - The child must leave with _exit(), not exit() and not return. exit()
 *     flushes this process's stdio buffers, which are a copy of the parent's,
 *     and you get the duplicated output from syscalls/buffering.c section 3.
 *     Use exit code 127 for "command not found" (ENOENT) and 126 for "found
 *     but could not run it" (EACCES) -- that is what shells report and what
 *     test-spawn.sh expects.
 *
 *   - waitpid can return -1 with errno == EINTR if a signal arrived while
 *     you were waiting. That is not the child exiting. Retry.
 *
 *   - execvp, not execv: the p versions search $PATH, so `ls` works and not
 *     just `/bin/ls`. The v versions take an argv array; the l versions take
 *     a variable argument list. `man 3 exec` has the naming table -- read it
 *     once and the six names stop looking arbitrary.
 *
 * argv must be NULL-terminated. execvp finds the end of the array that way
 * and has no other way to know.
 */
static int spawn(char **argv)
{
        (void)argv;
        /* Keeps -Wunused-function quiet while the stub is still a
         * stub. Delete this line once describe_status is called for real. */
        (void)describe_status;
        return -1;
}

int main(int argc, char **argv)
{
        int rc;

        if (argc < 2) {
                fprintf(stderr, "usage: %s command [args...]\n", argv[0]);
                return 2;
        }

        rc = spawn(&argv[1]);
        if (rc < 0)
                return 1;
        return rc;
}
