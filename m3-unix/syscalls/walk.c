/*
 * walk.c -- a tiny find(1): walk a directory tree and report what is in it.
 * Nathan Dearing, 2026-08-21.
 *
 * The other half of "files" -- mycat is about a file's contents, this is
 * about its metadata and about directories as a thing you read.
 *
 * A directory is a file. You cannot read() it (Linux returns EISDIR), so
 * there is a separate interface: opendir/readdir/closedir, in section 3,
 * built on the getdents system call in section 2. `man 3 readdir` and
 * `man 2 getdents` are worth reading back to back, once, to see a libc
 * wrapper next to the call it wraps.
 *
 * Verify by hand when it works -- the ordering will differ, so sort both:
 *     ./walk /usr/share/doc | awk '{print $3}' | sort > /tmp/mine
 *     find /usr/share/doc                      | sort > /tmp/theirs
 *     diff /tmp/mine /tmp/theirs
 *
 * (That awk used to be a sed with a regex in it, until the regex contained
 * the two characters that end a C comment and the file stopped compiling.
 * Worth knowing before it happens to you in a file you care about.)
 */

#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * TODO 1 -- type_char
 *
 * Return the one-character type for a mode, the way ls -l's first column
 * does it: '-' regular, 'd' directory, 'l' symlink, 'c' character device,
 * 'b' block device, 'p' fifo, 's' socket, '?' anything else.
 *
 * Use the S_IS* macros from <sys/stat.h> -- S_ISREG(m), S_ISDIR(m) and so
 * on. Do NOT compare st_mode against numbers: the type bits are a field
 * inside st_mode, not the whole of it, and `mode == S_IFDIR` is false for
 * every real directory because the permission bits are also in there.
 */
static char type_char(mode_t mode)
{
        (void)mode;
        return '?';
}

/*
 * TODO 2 -- report
 *
 * Print one line for `path`: type character, size in bytes, and the path.
 *
 *     d      4096 /usr/share/doc
 *     -       220 /usr/share/doc/README
 *
 * Use lstat, not stat. stat follows symlinks and reports the target; lstat
 * reports the link itself. A tree walker that uses stat will follow a link
 * pointing at its own ancestor and recurse until it runs out of path.
 *
 * Return 0 on success, -1 on failure with a message on stderr. A file that
 * vanishes between readdir and lstat is a normal race, not a crash.
 */
static int report(const char *path)
{
        (void)path;
        return -1;
}

/*
 * TODO 3 -- walk
 *
 * Report `path`, and if it is a directory, recurse into everything inside it.
 * Return the number of entries that could not be read.
 *
 * Four things this will get wrong on the first attempt:
 *
 *   - "." and ".." are in every directory readdir hands you. Recursing into
 *     them does not terminate.
 *   - readdir gives you a NAME, not a path. You have to join it onto `path`
 *     yourself, with a '/' unless path already ends in one. snprintf into a
 *     buffer of PATH_MAX and check the return for truncation.
 *   - readdir returns NULL both at the end of the directory and on error.
 *     To tell them apart: set errno = 0 before the call, and check it after.
 *     This is the one place in POSIX where that pattern is correct, and
 *     `man 3 readdir` says so explicitly.
 *   - A directory you cannot open (EACCES) should print a message and let
 *     the walk continue, not abort it. That is what find does.
 *
 * Every opendir needs a matching closedir. Run it under valgrind on a real
 * tree -- a leaked DIR * per directory adds up fast.
 */
static int walk(const char *path)
{
        (void)path;
        /* Keeps -Wunused-function quiet while the stub is still a
         * stub. Delete this line once type_char and report are called for real. */
        (void)type_char;
        (void)report;
        return 0;
}

int main(int argc, char **argv)
{
        int failed = 0;
        int i;

        if (argc < 2) {
                failed = walk(".");
        } else {
                for (i = 1; i < argc; i++)
                        failed += walk(argv[i]);
        }

        if (failed > 0) {
                fprintf(stderr, "walk: %d entr%s could not be read\n",
                        failed, failed == 1 ? "y" : "ies");
                return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
}
