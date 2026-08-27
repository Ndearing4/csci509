# The Makefile you have to write

There is no `build.sh` in this directory and there is no Makefile. Writing one
is the M2 exit test, and `check-makefile.sh` grades it.

M1's `build.sh` scripts ran the same three commands every time:

```sh
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c list.c      -o list.o
gcc -std=c11 -Wall -Wextra -Werror -g -O0 -c test_list.c -o test_list.o
gcc list.o test_list.o -o test_list
```

That is what you are describing to `make` — but *describing*, not scripting.
A Makefile is a set of "this file depends on those files, and here is how to
rebuild it" facts. `make` works out the order and skips work that is already
done. The difference shows the first time you change one file out of two.

## The rule skeleton, once, concretely

Every Makefile rule is this shape:

```makefile
target: prereq1 prereq2
	recipe-line-1
	recipe-line-2
```

`target` is the file to build, the prereqs are what it depends on, and the
recipe lines are shell commands that build it — indented with one literal
TAB, not spaces. `make` compares the target's timestamp against each
prerequisite's and reruns the recipe only if a prerequisite is newer, or the
target doesn't exist yet. Every rule you write below (`all`, `list.o`,
`test_list.o`, `test_list`, `clean`) is an instance of exactly this shape;
the numbered list is the parts of it that are easy to get wrong.

## What it must do

`check-makefile.sh` verifies all seven:

1. **`make` builds `./test_list`** from a clean tree.
2. **`make` twice in a row does nothing the second time.** If it recompiles or
   relinks when no source changed, your dependencies are wrong (or you have a
   target whose name is not the file it creates).
3. **Touching `list.h` rebuilds both object files.** Both `.c` files include
   it, so both depend on it. `make` cannot see `#include` — you have to say so.
4. **Touching `list.c` rebuilds only `list.o`,** then relinks. Not
   `test_list.o`. This is the payoff, and it is what a shell script cannot do.
5. **`make clean` removes the objects and the binary,** and leaves the sources
   alone.
6. **The compile flags are `-std=c11 -Wall -Wextra -Werror -g -O0`,** held in a
   `CFLAGS` variable rather than repeated on each line. `-g -O0` because you
   are about to spend hours in gdb and optimized code single-steps out of order.
7. **`clean` is declared `.PHONY`.** Ask yourself what breaks if someone
   creates a file named `clean` — then you will remember what `.PHONY` is for.

## What it must not do

- **No `%.o: %.c` pattern rule on the first pass, and no built-in implicit
  rules.** Write both compile lines out. Pattern rules are worth learning
  *after* you have written the long form once, because until then you cannot
  see what they are abbreviating.
- **No `wildcard`, no `shell`, no recursion.** Four files, three rules.
- **Do not copy one.** Every Makefile on the internet is either trivially
  wrong or solving a problem you do not have.

## The five things that trip people up

- **Recipes are indented with a literal TAB, not spaces.** Not four spaces, not
  eight. A space-indented recipe gives you `Makefile:7: *** missing separator.
  Stop.` and that message never mentions tabs. Your editor may be helping;
  turn that off for this file.
- **A rule's target should be the file it creates.** `list.o: list.c list.h`,
  not `compile: ...`. `make` checks whether the *target file* is older than its
  prerequisites; a target that never becomes a file is always out of date, so it
  always reruns — which is check 2 failing.
- **The first target in the file is the default.** Put `all: test_list` at the
  top, or `make` with no arguments will build whichever rule you happened to
  write first.
- **`$@` is the target, `$^` is all prerequisites, `$<` is the first one.**
  Three characters that make the recipes stop repeating filenames.
- **`make -n` prints what it *would* do without doing it.** Use it constantly
  while writing this. `make --debug=b` explains *why* it decided to rebuild
  something, which is the answer to "why does it keep recompiling".

## Working order

Write it in the order the checks are numbered. Get `make` to produce the
binary at all, then run `./check-makefile.sh` and fix whatever it names.
Check 4 is the one that will need you to actually think.

When it passes, read your own Makefile and answer: what happens if you delete
`list.h` from the prerequisite lists? Try it. Touch `list.h`, run `make`, and
watch it do nothing — then imagine that having been a changed `struct Node`.
That is the bug the header dependency exists to prevent, and it produces a
program where two `.o` files disagree about how big a node is.
