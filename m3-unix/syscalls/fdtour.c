/*
 * fdtour.c -- the system-call file API, demonstrated before you write mycat.
 * Nathan Dearing, 2026-08-21.
 *
 * The third tour, after m1-data/c-syntax and m2-memory/c-structs, and the
 * same deal: everything mycat.c needs is shown here on a DIFFERENT problem,
 * so assembling it is still your job. This program never copies a whole
 * file, which is the one thing mycat has to do.
 *
 * It creates its own scratch file in /tmp and deletes it at the end, so it
 * touches nothing you care about.
 *
 *   ./build.sh
 *   ./fdtour
 *
 * COMING FROM JAVA
 *
 * Java hands you FileInputStream, BufferedReader, try-with-resources, and an
 * IOException when something goes wrong. Every one of those is built on what
 * is in this file, and none of them exist here:
 *
 *   Java                          C, at this level
 *   ------------------------      ---------------------------------------
 *   new FileInputStream(path)     open(path, O_RDONLY)  -> an int
 *   in.read(buf)                  read(fd, buf, n)      -> a count, or -1
 *   catch (IOException e)         check the return value, then read errno
 *   e.getMessage()                strerror(errno)
 *   try-with-resources            close(fd), by hand, on every path
 *   BufferedReader                the stdio layer -- section 8, and
 *                                 buffering.c next door
 *
 * The one that catches people: there are no exceptions, so nothing is
 * thrown, so a call you do not check simply carries on with a bad value.
 * Every system call in this file is checked. That is not defensive style,
 * it is the only error mechanism there is.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static char scratch[] = "/tmp/fdtour.XXXXXX";   /* mkstemp rewrites the Xs */

/* Give up, loudly, naming the call that failed. Every section uses this so
 * the error handling does not drown out what is being demonstrated. */
static void die(const char *what)
{
        perror(what);
        exit(EXIT_FAILURE);
}

/* ------------------------------------------------------------------ *
 * 1. A file descriptor is a small integer. That is the whole idea.
 * ------------------------------------------------------------------ */
static void s1_what_is_an_fd(void)
{
        int fd;

        printf("--- 1. what a file descriptor is ---\n");

        printf("  STDIN_FILENO  = %d\n", STDIN_FILENO);
        printf("  STDOUT_FILENO = %d\n", STDOUT_FILENO);
        printf("  STDERR_FILENO = %d\n", STDERR_FILENO);

        fd = mkstemp(scratch);          /* creates the file and opens it */
        if (fd == -1)
                die("mkstemp");

        printf("  our scratch file %s opened as fd %d\n", scratch, fd);
        printf("\n");
        printf("  3, because 0, 1 and 2 were already taken. The kernel keeps a\n");
        printf("  table of open files PER PROCESS, and a descriptor is an index\n");
        printf("  into it -- it always hands out the lowest free one. There is\n");
        printf("  no object here and nothing to inspect; the state is in the\n");
        printf("  kernel and this int is your handle on it.\n");
        printf("\n");
        printf("  That 'lowest free one' rule is not trivia. It is how the\n");
        printf("  classic shell redirection trick works: close(1), then open a\n");
        printf("  file, and the file GETS descriptor 1. dup2 in section 7 is\n");
        printf("  the modern way to do the same thing on purpose.\n");

        if (close(fd) == -1)
                die("close");
}

/* ------------------------------------------------------------------ *
 * 2. write: it returns a count, and the count can surprise you.
 * ------------------------------------------------------------------ */
static void s2_write(void)
{
        const char *line = "the quick brown fox\n";
        int         fd;
        ssize_t     n;

        printf("--- 2. write ---\n");

        /* O_WRONLY: writing only. O_TRUNC: start from empty.
         * The flags are OR-ed together; each is one bit. */
        fd = open(scratch, O_WRONLY | O_TRUNC);
        if (fd == -1)
                die("open");

        n = write(fd, line, strlen(line));
        if (n == -1)
                die("write");

        printf("  asked to write %zu bytes, wrote %zd\n", strlen(line), n);
        printf("  ssize_t, not size_t -- it has to be able to hold -1.\n");
        printf("\n");
        printf("  On a regular file a short write basically never happens, and\n");
        printf("  that is exactly why it is dangerous: code that assumes it\n");
        printf("  works until the day the output is a pipe or a socket. If\n");
        printf("  write returns less than you asked, it has NOT failed -- it\n");
        printf("  has written that much and you owe it the rest. That is why\n");
        printf("  mycat's TODO 1 says the write is a loop inside the read loop.\n");

        if (close(fd) == -1)
                die("close");
}

/* ------------------------------------------------------------------ *
 * 3. read: three different return values, three different meanings.
 * ------------------------------------------------------------------ */
static void s3_read(void)
{
        char    buf[8];         /* deliberately tiny, so one read cannot finish */
        int     fd;
        ssize_t n;
        int     calls = 0;

        printf("--- 3. read ---\n");

        fd = open(scratch, O_RDONLY);
        if (fd == -1)
                die("open");

        /* THE loop. Three cases, and they are all different:
         *      n > 0   this many bytes are in buf. Maybe fewer than asked.
         *      n == 0  end of file. Not an error.
         *      n < 0   error, and errno says which.
         *
         * A `while (read(...))` that treats 0 and -1 the same is the single
         * most common beginner bug in this whole area, because it works
         * until something fails. */
        while ((n = read(fd, buf, sizeof buf)) > 0) {
                calls++;
                printf("  read #%d returned %2zd: \"", calls, n);
                fwrite(buf, 1, (size_t)n, stdout);
                printf("\"\n");
        }
        if (n == -1)
                die("read");

        printf("  read #%d returned 0 -- end of file\n", calls + 1);
        printf("\n");
        printf("  Note what is NOT in buf: a terminator. read moves bytes, it\n");
        printf("  does not make strings, so buf is not printable with %%s and\n");
        printf("  that is why the line above uses fwrite with an explicit\n");
        printf("  length. Nothing at this layer knows what a line is either.\n");

        if (close(fd) == -1)
                die("close");
}

/* ------------------------------------------------------------------ *
 * 4. lseek: the file offset lives in the kernel, not in your program.
 * ------------------------------------------------------------------ */
static void s4_lseek(void)
{
        char    buf[16];
        int     fd;
        ssize_t n;
        off_t   pos;

        printf("--- 4. lseek ---\n");

        fd = open(scratch, O_RDONLY);
        if (fd == -1)
                die("open");

        pos = lseek(fd, 0, SEEK_END);           /* how big is it? */
        if (pos == (off_t)-1)
                die("lseek");
        printf("  seeking to SEEK_END says the file is %lld bytes\n", (long long)pos);

        if (lseek(fd, 4, SEEK_SET) == (off_t)-1) /* absolute */
                die("lseek");
        n = read(fd, buf, 5);
        if (n == -1)
                die("read");
        printf("  from offset 4, 5 bytes: \"");
        fwrite(buf, 1, (size_t)n, stdout);
        printf("\"\n");

        pos = lseek(fd, 0, SEEK_CUR);           /* where am I now? */
        if (pos == (off_t)-1)
                die("lseek");
        printf("  the offset is now %lld -- read moved it, nothing else did\n",
               (long long)pos);
        printf("\n");
        printf("  There is no variable in this program holding that 9. The\n");
        printf("  offset is state attached to the OPEN FILE, kept by the\n");
        printf("  kernel, and every read and write advances it. Two processes\n");
        printf("  that inherited the same open file share the offset; two that\n");
        printf("  opened the same path separately do not. That distinction is\n");
        printf("  APUE chapter 3 and it comes back in the shell.\n");

        if (close(fd) == -1)
                die("close");
}

/* ------------------------------------------------------------------ *
 * 5. errno: the whole error-reporting mechanism, and its three rules.
 * ------------------------------------------------------------------ */
static void s5_errno(void)
{
        int fd;
        int saved;

        printf("--- 5. errno ---\n");

        /* Rule 1: check the return value. errno is only meaningful after a
         * call that FAILED -- a successful call may leave any old value in
         * it, so `if (errno)` on its own is always wrong. */
        errno = 0;
        fd = open("/nonexistent/path/at/all", O_RDONLY);
        printf("  open of a missing path returned %d\n", fd);
        if (fd == -1) {
                saved = errno;          /* Rule 3: save it immediately */
                printf("  errno = %d, strerror says: %s\n", saved, strerror(saved));
                fflush(stdout);         /* see the note below */
                perror("  perror prints");
                printf("  (that fflush is not decoration. perror writes to\n");
                printf("   stderr, which is never buffered, so without it the\n");
                printf("   line jumps ahead of everything still sitting in\n");
                printf("   stdout's buffer the moment you pipe this program\n");
                printf("   into anything. buffering.c section 4, met early.)\n");
        }

        /* Rule 2: different failures, different errno. The message is worth
         * showing rather than guessing at. */
        fd = open("/etc", O_WRONLY);
        if (fd == -1)
                printf("  opening a directory for writing: %s\n", strerror(errno));

        fd = open("/etc/shadow", O_RDONLY);
        if (fd == -1)
                printf("  opening a root-only file:        %s\n", strerror(errno));
        else
                close(fd);              /* running as root, apparently */

        /* Rule 3, demonstrated. printf is a library call and library calls
         * are allowed to set errno, so anything between the failure and the
         * inspection can destroy the evidence. */
        errno = 0;
        if (open("/nonexistent/again", O_RDONLY) == -1) {
                saved = errno;
                printf("  saved errno before doing anything else: %d\n", saved);
                printf("  ... and errno right now is %d\n", errno);
                printf("  (they match here, but they are not guaranteed to.\n");
                printf("   Save it in the line after the failure, every time.)\n");
        }

        printf("\n");
        printf("  This is the entire error mechanism. No exception, no stack\n");
        printf("  trace, no compiler warning if you ignore it. `man 3 errno`\n");
        printf("  lists the names; `man 2 open` has the ERRORS section saying\n");
        printf("  which ones that call can produce and what each one means.\n");
        printf("  Reading the ERRORS section of a man page is a habit worth\n");
        printf("  building now.\n");
}

/* ------------------------------------------------------------------ *
 * 6. fstat: everything about a file except its contents.
 * ------------------------------------------------------------------ */
static void s6_fstat(void)
{
        struct stat st;
        int         fd;

        printf("--- 6. stat ---\n");

        fd = open(scratch, O_RDONLY);
        if (fd == -1)
                die("open");

        /* fstat takes an fd; stat takes a path; lstat takes a path and does
         * not follow symlinks. Same struct out of all three. */
        if (fstat(fd, &st) == -1)
                die("fstat");

        printf("  size         %lld bytes\n", (long long)st.st_size);
        printf("  inode        %llu\n", (unsigned long long)st.st_ino);
        printf("  links        %llu\n", (unsigned long long)st.st_nlink);
        printf("  mode         %o (octal)\n", (unsigned)(st.st_mode & 07777));
        printf("  is a regular file?   %s\n", S_ISREG(st.st_mode) ? "yes" : "no");
        printf("  is a directory?      %s\n", S_ISDIR(st.st_mode) ? "yes" : "no");
        printf("\n");
        printf("  st_mode packs the TYPE and the PERMISSIONS into one number,\n");
        printf("  which is why you use S_ISREG(m) rather than comparing m to\n");
        printf("  something. `m == S_IFREG` is false for every real file,\n");
        printf("  because the permission bits are in there too. That is\n");
        printf("  walk.c TODO 1.\n");

        if (close(fd) == -1)
                die("close");
}

/* ------------------------------------------------------------------ *
 * 7. dup2: how redirection works. The shell's stage 2, in miniature.
 * ------------------------------------------------------------------ */
static void s7_dup2(void)
{
        int fd;
        int saved_stdout;

        printf("--- 7. dup2 ---\n");
        printf("  the next three printfs go into %s, not here\n", scratch);
        fflush(stdout);         /* flush BEFORE moving stdout out from under it */

        /* Keep a spare copy of the current stdout so we can put it back.
         * dup returns a new descriptor onto the same open file. */
        saved_stdout = dup(STDOUT_FILENO);
        if (saved_stdout == -1)
                die("dup");

        fd = open(scratch, O_WRONLY | O_TRUNC);
        if (fd == -1)
                die("open");

        /* dup2(old, new): make `new` refer to the same open file as `old`,
         * closing whatever `new` was. After this, descriptor 1 IS the file. */
        if (dup2(fd, STDOUT_FILENO) == -1)
                die("dup2");
        if (close(fd) == -1)            /* the spare copy is not needed */
                die("close");

        printf("this line went to the file\n");
        printf("so did this one\n");
        printf("and this one\n");
        fflush(stdout);

        /* Put stdout back. */
        if (dup2(saved_stdout, STDOUT_FILENO) == -1)
                die("dup2");
        if (close(saved_stdout) == -1)
                die("close");

        printf("  ... and we are back. What landed in the file, read as\n");
        printf("  raw bytes -- note it arrives as ONE blob, not three lines:\n");

        {
                char    buf[128];
                ssize_t n;

                fd = open(scratch, O_RDONLY);
                if (fd == -1)
                        die("open");
                while ((n = read(fd, buf, sizeof buf)) > 0) {
                        printf("    [%zd bytes] ", n);
                        fwrite(buf, 1, (size_t)n, stdout);
                }
                if (n == -1)
                        die("read");
                if (close(fd) == -1)
                        die("close");
        }

        printf("\n");
        printf("  Nothing in printf changed. printf writes to descriptor 1 and\n");
        printf("  always did; we changed what descriptor 1 MEANS. That is why\n");
        printf("  `ls > out.txt` needs no redirection code inside ls, and why\n");
        printf("  redirection is a shell feature rather than a kernel one.\n");
        printf("\n");
        printf("  In the shell you will do this between fork and exec, in the\n");
        printf("  CHILD, and then you do not need to save and restore anything\n");
        printf("  -- the child is about to be replaced by another program. Do\n");
        printf("  it in the parent, as here, and you must put it back, which\n");
        printf("  this section does only so it can keep talking to you.\n");
}

/* ------------------------------------------------------------------ *
 * 8. The same file through stdio, for contrast.
 * ------------------------------------------------------------------ */
static void s8_stdio(void)
{
        FILE *f;
        char  line[128];

        printf("--- 8. the stdio layer on top ---\n");
        printf("  the same file again, and this time it comes out a line at\n");
        printf("  a time, each one terminated:\n");

        f = fopen(scratch, "r");        /* returns NULL on failure, not -1 */
        if (f == NULL)
                die("fopen");

        while (fgets(line, sizeof line, f) != NULL)
                printf("    | %s", line);

        if (ferror(f))                  /* errors are checked separately */
                die("fgets");
        if (fclose(f) == EOF)
                die("fclose");

        printf("\n");
        printf("  Same bytes, and a completely different interface:\n");
        printf("    open  -> an int          fopen  -> a FILE *, or NULL\n");
        printf("    read  -> raw bytes       fgets  -> a LINE, terminated\n");
        printf("    errno every time         ferror/feof, and sometimes errno\n");
        printf("    every call is a syscall  buffered; one syscall per 4096\n");
        printf("\n");
        printf("  fgets gives you a line and a '\\0'. read gives you neither.\n");
        printf("  That convenience is the whole reason stdio exists, and the\n");
        printf("  buffering that makes it fast is the reason its output can\n");
        printf("  come out in an order that surprises you. Run\n");
        printf("  ./show-buffering.sh next -- that is what it is about.\n");
        printf("\n");
        printf("  `man 2 read` and `man 3 fgets`. Section 2 and section 3.\n");
        printf("  You now have a concrete reason to care which is which.\n");
}

int main(void)
{
        s1_what_is_an_fd(); putchar('\n');
        s2_write();         putchar('\n');
        s3_read();          putchar('\n');
        s4_lseek();         putchar('\n');
        s5_errno();         putchar('\n');
        s6_fstat();         putchar('\n');
        s7_dup2();          putchar('\n');
        s8_stdio();

        if (unlink(scratch) == -1)
                die("unlink");
        printf("\n(removed %s)\n", scratch);
        return EXIT_SUCCESS;
}
