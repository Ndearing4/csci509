# M2 — C memory and tooling · 8–14h

**Skip-ahead probe:** write a linked-list insert taking `Node **head`. Why the
double pointer?

Instant answer, with a correct one-line reason → skip to M3. Otherwise `list/`
is built around exactly that question.

## The three exercises

```
list/       a linked list across .h/.c, and the Makefile you write by hand
strings/    C strings: null termination, strcpy vs strncpy, off-by-one
bugs/       six planted memory bugs, to be found in gdb before valgrind
```

Order matters here, unlike M1. `list/` gives you a heap program worth
debugging; `bugs/` teaches you to debug it. Do `strings/` whenever you want a
break from the list — it is independent.

The tooling half is the part that transfers. Every module after this one
assumes you can set a breakpoint, read a backtrace, and write a Makefile
without copying one.

### list/

```sh
cd list
# there is no build.sh here. Writing the Makefile is the exercise.
make            # once you have written one
./check-makefile.sh
```

`list.h` declares nine functions and documents what each must do. `list.c` has
the stubs. `test_list.c` is a harness that checks all nine, including the
cases people get wrong: removing the *first* element, removing from an empty
list, popping the last element.

Five of the nine take `Node **head`. Work out why before you implement them —
write `list_push_front` with a single `Node *head` parameter first, watch the
caller's pointer not change, and *then* fix it. That failure is worth two
paragraphs of explanation.

`MAKEFILE.md` states what the Makefile has to do and `check-makefile.sh`
verifies it — including the two things a first Makefile usually gets wrong:
depending on the header, and not relinking when nothing changed.

**Do not copy a Makefile from anywhere.** The whole point is that after this
you can read one.

### strings/

```sh
cd strings
./build.sh
```

Six functions, each a re-implementation of something in `<string.h>`. The
harness compares yours against libc's where the semantics are supposed to
match, and separately checks the cases where `strncpy` does something people
do not expect.

The trap is puzzle 3. `strncpy(dst, src, n)` does **not** guarantee a
terminated string, and it *does* zero-fill the whole remaining buffer when the
source is short. Both halves of that surprise people. The harness plants a
canary byte after the destination so you can see it.

### bugs/

```sh
cd bugs
./build.sh          # builds all six with -g -O0
```

Six programs, one planted memory bug each. All six build clean under
`-Wall -Wextra -Werror`, which is the first lesson: **the compiler does not
find these.** That is why gdb and valgrind exist.

The workflow is in `HUNT.md`, and it has a table to fill in. The order is
fixed and it is the point of the exercise:

1. Read the source and **predict** the bug.
2. Find it in **gdb**. Write down the line number and the actual bad value.
3. *Then* run **valgrind** and see whether it agrees with you.

Running valgrind first turns this into an exercise in reading tool output.
Running gdb first turns it into an exercise in debugging, which is the skill
being built.

## Exit test

- [ ] `make` builds `list/` from a Makefile you wrote, and `./check-makefile.sh`
      reports clean
- [ ] `./test_list` passes all nine groups, and `valgrind --leak-check=full
      ./test_list` reports zero leaks and zero errors
- [ ] `./build.sh` in `strings/` passes, including the three truncation cases
- [ ] All six bugs in `bugs/` located in gdb, with the line number written down
      **before** valgrind confirmed it
- [ ] You can explain the double pointer without hedging

## gdb, the twelve commands that cover it

Everything downstream assumes these. There is no exercise for them — use them
on `bugs/` until you stop looking them up.

| | |
|---|---|
| `break file.c:42`, `break func` | stop somewhere |
| `run`, `run args...` | start |
| `next` / `step` | over a call / into a call |
| `finish` | run until this frame returns |
| `continue` | until the next breakpoint |
| `print expr`, `print *ptr`, `print arr[3]` | evaluate anything |
| `backtrace` (`bt`) | how did I get here |
| `frame N`, `up`, `down` | move around the stack |
| `x/16xb ptr` | dump raw memory, 16 bytes, hex |
| `info locals`, `info args` | what is in scope |
| `watch var` | stop when it changes — the one that finds corruption |
| `list` | show source around here |

`.gdbinit` in your home directory already sets history and pretty-printing;
nothing here depends on it.

## valgrind, the four errors you will see

| Message | Means |
|---|---|
| `Invalid read/write of size N` | you touched memory you do not own |
| `Invalid free() / delete` | freeing something not returned by malloc, or freeing twice |
| `N bytes in 1 blocks are definitely lost` | leak, and nothing points at it any more |
| `Conditional jump depends on uninitialised value(s)` | you read memory you never wrote |

Always `valgrind --leak-check=full --track-origins=yes`. Without
`--track-origins` the uninitialised-value message tells you where it was
*used*, which is rarely where it went wrong.

## Undefined behavior to stay clear of

- Reading a freed pointer. It usually "works" — the memory is still mapped and
  often still holds the old value. `bugs/bug1` is built on that.
- `free`ing the same pointer twice. Corrupts the allocator's own bookkeeping,
  so the crash lands somewhere unrelated. This is why the crash site is not the
  bug site.
- Reading uninitialized heap memory. `malloc` does not zero; `calloc` does.
- Writing one past the end of an allocation. The classic `<=` in a loop.
- Using a pointer to a local after its block ends. The stack frame is reused
  by the *next* call, so the value survives until it suddenly does not.

*Stretch, revisit after M8:* write your own `malloc` on top of `sbrk`. Best
single exercise for understanding what the heap actually is. Skip on the first
pass — see the note in the top-level README.
