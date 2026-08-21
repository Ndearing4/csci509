# M3 — UNIX systems programming · 10–16h

**Skip-ahead probe:** `write(1, buf, n)` and `fwrite(buf, 1, n, stdout)` put the
same bytes in the same place. Name three ways they behave differently, and say
which one of them is a system call.

Instant answer → skim `syscalls/buffering.c` and go to `procs/`. Otherwise this
module starts exactly there.

This is the 347 gap and the assumed background whoever teaches 509. Everything
here is the interface between a program and the kernel, which is the subject of
the course.

**Reading: APUE ch 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15.2–15.3.** Read the
chapter for a section before doing it, not after.

## The three exercises, in order

```
syscalls/   file descriptors, errno, stdio vs raw I/O, stat and directories
procs/      fork, exec, wait, exit status, zombies and orphans
shell/      the project: a shell with pipes, redirection, and Ctrl-C
```

`shell/` needs both of the others. Do not start it early — the four stages in
`shell/STAGES.md` assume you have already written `spawn.c`.

## `man 2` versus `man 3`

This is exam material and it costs nothing to internalise now.

| | |
|---|---|
| `man 2 write` | **section 2, system calls.** A trap into the kernel. |
| `man 3 fwrite` | **section 3, library functions.** Ordinary user-space code. |

`man 2 intro` and `man 3 intro` say it in the source's own words. When a name
lives in both — `printf` does not, but `open` and `fopen` are the same idea —
`man 2 open` and `man 3 fopen` are answering different questions.

The practical version: a section 2 page sets `errno` and returns -1. A section
3 page might do that, might return NULL, might return a count, and its manual
page will say which. Never guess; the "RETURN VALUE" section exists.

Get in the habit of `man -f name` (or `whatis name`) when you are not sure
which section something is in.

## errno, and the thing everyone gets wrong

```c
fd = open(path, O_RDONLY);
if (fd == -1) {
        fprintf(stderr, "open %s: %s\n", path, strerror(errno));
        return 1;
}
```

Three rules, and the harnesses here check all three:

1. **Check every syscall.** `read` returning a short count is normal, not an
   error. `write` returning less than you asked is normal too.
2. **Only look at `errno` after a call that failed.** It is not cleared on
   success, so reading it after a call that worked gives you a stale value
   from something that failed ten minutes ago.
3. **Save it immediately if you are going to do anything else first.** Any
   library call in between — including `printf` — may overwrite it.

`perror("open")` is the two-second version of that fprintf. Both are fine.

## Exit test

- [ ] `./test-mycat.sh` passes — your `mycat` matches `cat` byte for byte,
      including on a file it cannot read, where it must match cat's *exit
      status* and put its message on stderr
- [ ] You can explain every line of `buffering.c`'s output before running it,
      including which of the three forks duplicates its line, in which of
      the two modes, and why 3b differs between them
- [ ] `./test-spawn.sh` passes, including the two signal cases
- [ ] `ls -la | grep foo > out.txt` works in your shell
- [ ] Ctrl-C kills the running child, not the shell
- [ ] `./test-shell.sh` passes, and `ps` shows no zombies after a hundred
      commands

## Things that will bite

- **Forgetting the feature-test macro.** Every file here starts with
  `#define _POSIX_C_SOURCE 200809L` before the first `#include`. Under
  `-std=c11` glibc hides everything POSIX by default, and the symptom is
  `implicit declaration of function 'fork'` — which under `-Werror` is an
  error, and which people fix by adding the wrong header. See the comment at
  the top of `syscalls/mycat.c`.
- **Not looping on `read` and `write`.** Both are allowed to transfer less
  than you asked for, and on a pipe or a terminal they routinely do.
- **`fork` returning three ways.** 0 in the child, the child's pid in the
  parent, -1 on failure. Test for -1 first; a program that treats -1 as "I am
  the child" forks the parent's whole world in half.
- **Forgetting `waitpid`.** Every child you do not reap stays a zombie in the
  process table. A shell that leaks one per command is a shell that dies.
- **Not closing pipe ends.** A reader that never sees EOF is almost always a
  writer end still open *in the reading process*. This is the single most
  common shell bug and `shell/STAGES.md` stage 3 is mostly about it.
- **Doing real work in a signal handler.** Only `volatile sig_atomic_t` writes
  and async-signal-safe calls. `printf` is not one of them; `write` is.
  `man 7 signal-safety` has the whole list.
