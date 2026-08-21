/*
 * buffering.c -- what the stdio layer is doing on top of the file descriptor.
 * Nathan Dearing, 2026-08-21.
 *
 * Not a stub. This one is complete, and the exercise is to predict every line
 * of its output -- twice, because the output changes depending on whether
 * stdout is a terminal or a pipe. `./show-buffering.sh` runs it both ways
 * side by side.
 *
 * The one fact underneath every section: printf does not write anything.
 * It copies bytes into a buffer inside your process and, sometimes, later,
 * calls write(). `write` is the system call; `printf` is a library function
 * that eventually causes one. That is the M3 skip-ahead probe.
 *
 * When stdio decides to actually call write():
 *
 *   - stdout is a terminal      -> line buffered. Flushes on every '\n'.
 *   - stdout is a pipe or file  -> fully buffered, 4096 bytes at a time.
 *   - stderr                    -> unbuffered. Always. This is why error
 *                                  messages survive a crash and printf
 *                                  output does not.
 *
 * Nobody chose that on purpose for your program. It is a heuristic in libc,
 * decided by isatty(), and it is the reason a program's output order changes
 * when you pipe it into `less`.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* write(2) directly, no stdio involved. Returns nothing useful on purpose --
 * this is a demo, and a real one would loop. See mycat.c TODO 1. */
static void raw(const char *s)
{
        ssize_t n = write(STDOUT_FILENO, s, strlen(s));

        (void)n;
}

int main(void)
{
        /* raw(), not printf -- a printf here would be buffered along with
         * everything else and would not appear until the end, which is
         * funny once and confusing every time after that. */
        raw(isatty(STDOUT_FILENO) ? "stdout is a terminal (line buffered)\n\n"
                                  : "stdout is NOT a terminal (fully buffered)\n\n");

        /* ---------------------------------------------------------------- */
        raw("=== 1. interleaving printf and write ===\n");
        printf("  A: printf\n");
        raw("  B: write\n");
        printf("  C: printf\n");
        raw("  D: write\n");
        raw("  ^ to a terminal: A B C D. Through a pipe: B D A C.\n");
        raw("    Same program, same lines, different order. The printfs sat\n");
        raw("    in a buffer until exit while the writes went straight out.\n\n");

        /* ---------------------------------------------------------------- */
        raw("=== 2. fflush is the fix ===\n");
        printf("  E: printf");
        fflush(stdout);
        raw(" then write on the same line\n");
        raw("  ^ E stays in place either way, because fflush forced the\n");
        raw("    buffer out before the write. That is the whole mechanism.\n\n");

        /* ---------------------------------------------------------------- */
        raw("=== 3a. fork with a dirty buffer ===\n");
        {
                pid_t pid;

                /* No newline, so nothing flushes here in EITHER mode -- a
                 * line-buffered stdout flushes on '\n', and there is not
                 * one. The bytes are sitting in this process's buffer. */
                printf("  F: printed once by the program");

                pid = fork();
                if (pid == -1) {
                        perror("fork");
                        return EXIT_FAILURE;
                }
                if (pid == 0) {
                        /* The child inherited a COPY of the buffer, dirty
                         * bytes and all. When it exits, its libc flushes. */
                        printf(" [child]\n");
                        exit(EXIT_SUCCESS);
                }
                if (waitpid(pid, NULL, 0) == -1) {
                        perror("waitpid");
                        return EXIT_FAILURE;
                }
                printf(" [parent]\n");
                fflush(stdout);
        }
        raw("  ^ F appears TWICE, terminal or pipe. fork copies the address\n");
        raw("    space, and an unflushed stdio buffer is in the address\n");
        raw("    space. Both copies flush when their process exits.\n\n");

        /* ---------------------------------------------------------------- */
        raw("=== 3b. the same thing, with a newline ===\n");
        {
                pid_t pid;

                printf("  G: printed once by the program\n");

                pid = fork();
                if (pid == -1) {
                        perror("fork");
                        return EXIT_FAILURE;
                }
                if (pid == 0)
                        exit(EXIT_SUCCESS);   /* exit, not _exit -- see below */
                if (waitpid(pid, NULL, 0) == -1) {
                        perror("waitpid");
                        return EXIT_FAILURE;
                }
                /* Flush here too, so the parent's copy lands next to the
                 * child's instead of surfacing under a later heading. */
                fflush(stdout);
        }
        raw("  ^ THIS one differs between the two runs. To a terminal G\n");
        raw("    appears once: line buffering flushed on the newline, so\n");
        raw("    the buffer was already clean at fork time. Through a pipe\n");
        raw("    it appears twice, because full buffering does not care\n");
        raw("    about newlines.\n");
        raw("    Same source, same fork, and whether it is a bug depends on\n");
        raw("    what the output happened to be connected to. That is how it\n");
        raw("    passes testing and then duplicates lines in production.\n\n");

        /* ---------------------------------------------------------------- */
        raw("=== 3c. the fix ===\n");
        {
                pid_t pid;

                printf("  H: printed once by the program");
                fflush(NULL);           /* flush every open stream, then fork */

                pid = fork();
                if (pid == -1) {
                        perror("fork");
                        return EXIT_FAILURE;
                }
                if (pid == 0)
                        exit(EXIT_SUCCESS);   /* exit, not _exit -- see below */
                if (waitpid(pid, NULL, 0) == -1) {
                        perror("waitpid");
                        return EXIT_FAILURE;
                }
                printf("\n");
                fflush(stdout);
        }
        raw("  ^ H appears once in both runs. fflush(NULL) flushes every\n");
        raw("    stream, so there is nothing left to duplicate. Any program\n");
        raw("    that forks and uses stdio needs that call before the fork,\n");
        raw("    and forgetting it is why duplicated log lines are a classic\n");
        raw("    UNIX bug.\n\n");
        raw("    Note what the child calls in all three: exit(), not _exit().\n");
        raw("    _exit() goes straight to the kernel and skips the stdio\n");
        raw("    flush entirely, so a child that leaves that way never\n");
        raw("    duplicates anything. That is exactly why procs/spawn.c\n");
        raw("    tells you to use _exit() in a failed exec path -- and it is\n");
        raw("    a workaround, not the fix. The fix is fflush(NULL).\n\n");

        /* ---------------------------------------------------------------- */
        raw("=== 4. stderr is not buffered ===\n");
        printf("  I: this printf goes to stdout\n");
        fprintf(stderr, "  J: this fprintf goes to stderr\n");
        raw("  ^ redirect them into the same file (2>&1) and J lands before\n");
        raw("    I, even though I ran first. stderr is unbuffered by the\n");
        raw("    standard, precisely so a message survives the crash that\n");
        raw("    would have eaten a buffered one.\n\n");

        raw("=== how to see the syscalls themselves ===\n");
        raw("  strace -e trace=write ./buffering\n");
        raw("  strace -e trace=write ./buffering | cat\n");
        raw("  Count the write() calls in each. That number is the entire\n");
        raw("  reason the stdio layer exists: it trades ordering surprises\n");
        raw("  for not trapping into the kernel once per printf.\n");

        return EXIT_SUCCESS;
}
