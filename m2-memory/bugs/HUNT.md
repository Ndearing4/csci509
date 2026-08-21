# Six bugs

Six programs, one planted memory bug each. All six compile clean under
`-Wall -Wextra -Werror`. Four of the six run to completion and print
plausible output.

```sh
./build.sh
```

## The order is the exercise

For each program, in this order, and write the answer down before moving on:

1. **Read the source. Predict the bug** — which line, and what goes wrong.
2. **Find it in gdb.** Not "run it and see it crash" — set a breakpoint,
   inspect the pointer, and be able to say what the actual bad value was.
3. **Only then run valgrind** and see whether it agrees with you.

Doing 3 first is easy and teaches you to read valgrind output. Doing 2 first
teaches you to debug, which is the skill that survives into a course where the
thing you are debugging is a kernel and valgrind is not available.

The comment block at the top of each file has a suggested gdb route. Try your
own first; read theirs when you are stuck for more than ten minutes.

## The table

Fill it in. The last column is the point: you are calibrating your own
prediction against a tool, and finding out where each tool is blind.

| | bug | predicted line | found in gdb at | valgrind says | agreed? |
|---|---|---|---|---|---|
| 1 | use after free | | | | |
| 2 | double free | | | | |
| 3 | leak | | | | |
| 4 | heap overflow | | | | |
| 5 | uninitialised read | | | | |
| 6 | dangling stack pointer | | | | |

Two rows are not like the others, and noticing that is most of the value here:

- **Bug 3 has no gdb answer.** Nothing goes wrong at any instant. There is no
  moment to break on. This is the case valgrind exists for, and the file says
  so — invert the order for that one.
- **Bug 6's valgrind answer is wrong.** It reports "uninitialised value",
  which is a different bug with a different fix, blames a frame it can only
  print as `???`, and fires on the line that prints the *correct* string.
  `gcc -fsanitize=address` names it properly � but only when asked:
  `ASAN_OPTIONS=detect_stack_use_after_return=1`, since that check is off by
  default. Plain ASan is silent. Run all three and compare.

Between them those two say the real thing: neither tool is a checklist you can
run instead of thinking. One of them is silent on a real bug and the other is
confidently wrong about one, and the only defense is having a prediction of
your own to check the output against. That is why column 3 comes before
column 5.

## Commands

```sh
gdb ./bug1_uaf                       # then: break run_all / run / next / print t
valgrind --leak-check=full --track-origins=yes ./bug1_uaf
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -fsanitize=address bug6_dangling.c -o bug6_asan
ASAN_OPTIONS=detect_stack_use_after_return=1 ./bug6_asan
```

`--track-origins=yes` costs runtime and is worth it: without it, an
uninitialised-value report tells you where the value was *used*, which is
usually nowhere near where it should have been written.

## After

Two things that are worth more than the six bugs:

**Plant one yourself.** Go to `../list/list.c`, break `list_free` so it reads
`node->next` after freeing `node`, and find it. Your own bug in your own code
under gdb is the exit test — the six here are practice for that.

**Fix them.** Each of the six has a one-line fix, and for two of them the
one-line fix is not the right fix:

- Bug 2's real fix is a sentence in a comment saying who owns the buffer. A
  double free is always two owners, and deleting one `free` just moves the
  question.
- Bug 6's real fix is not `static char name[32]` — that "works" and breaks the
  moment two reports exist at once. The fix is deciding where the storage
  lives: the caller's buffer, or the heap with a documented owner.
