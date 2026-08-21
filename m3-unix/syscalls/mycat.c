/*
 * mycat.c -- cat, written against the raw system calls.
 * Nathan Dearing, 2026-08-21.
 *
 * FEATURE TEST MACRO. This has to come before every #include in the file,
 * and every file in M3 has it:
 *
 *     #define _POSIX_C_SOURCE 200809L
 *
 * Under -std=c11 the compiler defines __STRICT_ANSI__, and glibc's headers
 * respond by hiding everything that is not in the C standard -- which is all
 * of POSIX, including open, read, write and close. The symptom is
 *
 *     error: implicit declaration of function 'read'
 *
 * on a file that already includes <unistd.h>, and the usual wrong fix is to
 * go hunting for a different header. The right fix is to say which standard
 * you want. 200809L is POSIX.1-2008. See feature_test_macros(7).
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 4096

/*
 * TODO 1 -- copy_fd
 *
 * Copy everything from `in` to `out`. Return 0 on success, -1 on error with
 * errno set by whichever call failed.
 *
 * The two things this exercise exists to teach:
 *
 *   - read() returns the number of bytes it actually read, which may be less
 *     than BUFSIZE for reasons that are not errors. 0 means end of file. -1
 *     means error. Three cases, and they are all different.
 *
 *   - write() may also transfer less than you asked. A write of n bytes that
 *     returns n-1 has not failed; it has written n-1 bytes and you owe it the
 *     rest. So the write is a loop inside the read loop. Getting this wrong
 *     is invisible on a regular file and shows up the first time the output
 *     is a pipe, which is to say the first time someone runs `mycat f | less`.
 *
 * Also handle EINTR: if a signal arrives mid-call, read/write return -1 with
 * errno == EINTR and nothing has gone wrong. Retry. `man 7 signal` explains
 * when that can happen.
 */
static int copy_fd(int in, int out)
{
        (void)in;
        (void)out;
        return -1;
}

/*
 * TODO 2 -- main
 *
 * With no arguments, copy stdin to stdout. With arguments, copy each named
 * file in turn; "-" means stdin.
 *
 * Match cat's behavior on errors, because test-mycat.sh diffs against the
 * real cat:
 *
 *   - a message on STDERR, not stdout, naming the file and the reason
 *   - keep going with the remaining files
 *   - exit status 1 if any file failed, 0 if none did
 *
 * strerror(errno) gives the reason text. perror() is the short version but
 * writes "prefix: reason" with a prefix you choose -- read what real cat
 * prints for a missing file and match its shape.
 *
 * Do NOT use fopen/fread here. Section 3 is the next exercise; this one is
 * section 2 only. The point of the pair is the difference.
 */
int main(int argc, char **argv)
{
        (void)argc;
        (void)argv;
        /* Keeps -Wunused-function quiet while the stub is still a
         * stub. Delete this line once copy_fd is called for real. */
        (void)copy_fd;

        fprintf(stderr, "mycat: not implemented yet\n");
        return EXIT_FAILURE;
}
