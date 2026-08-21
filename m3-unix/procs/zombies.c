/*
 * zombies.c -- watch a zombie and an orphan happen, on purpose.
 * Nathan Dearing, 2026-08-21.
 *
 * Complete, not a stub. Run it and follow the instructions it prints.
 *
 * Two states people learn as vocabulary and never see:
 *
 *   ZOMBIE   the child has exited, the parent has not called wait() yet.
 *            The process is gone -- no memory, no descriptors, nothing
 *            running -- but its entry stays in the process table holding
 *            its exit status, because nobody has collected it. It shows as
 *            Z or <defunct> in ps. It cannot be killed; it is already dead.
 *            kill -9 on a zombie does nothing, which surprises everyone once.
 *
 *   ORPHAN   the parent exited first. The child gets reparented, to init
 *            (pid 1) or to the nearest ancestor marked as a subreaper, and
 *            that new parent reaps it. An orphan is not a leak.
 *
 * So the leak is the zombie, and it is the parent's fault, and the fix is
 * always the same: wait for your children. A shell that forgets fills the
 * process table and then nothing on the machine can fork.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
        pid_t child;
        pid_t self = getpid();

        printf("=== 1. a zombie ===\n");
        printf("parent pid is %ld\n", (long)self);

        child = fork();
        if (child == -1) {
                perror("fork");
                return EXIT_FAILURE;
        }
        if (child == 0) {
                /* Child: exit immediately, and nobody reaps it yet. */
                _exit(3);
        }

        printf("child pid is %ld -- it has already exited\n", (long)child);
        printf("\nIn ANOTHER terminal, right now:\n");
        printf("    ps -o pid,ppid,stat,comm -p %ld\n", (long)child);
        printf("    ps aux | grep defunct\n");
        printf("STAT is Z. Try to kill it:\n");
        printf("    kill -9 %ld       <- does nothing. It is already dead.\n",
               (long)child);
        printf("\nPress Enter to have the parent call wait().\n");
        getchar();

        {
                int status;

                if (waitpid(child, &status, 0) == -1) {
                        perror("waitpid");
                        return EXIT_FAILURE;
                }
                printf("reaped %ld, exit status %d -- now check ps again,\n",
                       (long)child, WIFEXITED(status) ? WEXITSTATUS(status) : -1);
                printf("the process table entry is gone.\n\n");
        }

        printf("=== 2. an orphan ===\n");
        child = fork();
        if (child == -1) {
                perror("fork");
                return EXIT_FAILURE;
        }
        if (child == 0) {
                printf("child %ld: my parent is %ld\n",
                       (long)getpid(), (long)getppid());
                fflush(stdout);
                sleep(2);       /* outlive the parent */
                printf("child %ld: my parent is now %ld",
                       (long)getpid(), (long)getppid());
                printf("  <- reparented, the original is gone\n");
                fflush(stdout);
                _exit(EXIT_SUCCESS);
        }

        printf("parent %ld: exiting immediately, before the child\n",
               (long)self);
        printf("(your shell prompt comes back first -- the child keeps\n");
        printf(" printing into the terminal afterwards, which is what an\n");
        printf(" orphan looks like from the outside)\n");
        fflush(stdout);

        return EXIT_SUCCESS;
}
