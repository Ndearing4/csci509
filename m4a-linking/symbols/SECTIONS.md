# Which section does it land in?

Fill this in on paper — or in this file, it is yours — **before** running
`./reveal.sh`. Twelve variables and three non-variables from `sections.c`.

The four questions, in order, get you every answer:

1. Is it code? → `.text`
2. Is it a local, non-static variable? → no section at all; it lives on the
   stack, and it has no symbol because the linker never needs to know about it
3. Is it `const` with no address in it? → `.rodata`
4. Is its initial value all zeros (or absent)? → `.bss`, otherwise `.data`

| | declaration | your prediction | `nm` letter | actual section |
|---|---|---|---|---|
| 1 | `int alpha = 42;` | | | |
| 2 | `int bravo;` | | | |
| 3 | `int charlie = 0;` | | | |
| 4 | `const int delta = 7;` | | | |
| 5 | `static int echo_ = 9;` | | | |
| 6 | `static int foxtrot;` | | | |
| 7 | `char golf[1024];` | | | |
| 8 | `char hotel[1024] = "text";` | | | |
| 9 | `const char *india = "...";` | | | |
| 10 | `const char juliet[] = "...";` | | | |
| 11 | `int *kilo = NULL;` | | | |
| 12 | `int (*lima)(int,int) = NULL;` | | | |
| 13 | `int mike(int,int)` | | | |
| 14 | `static int novemb` (local) | | | |
| 15 | `int oscar` (local) | | | |

## Five that catch people

- **2 vs 3.** `int bravo;` and `int charlie = 0;` are written differently and
  end up identically. Explicitly initialising to zero costs nothing.
- **7 vs 8.** Both are `char[1024]`. One of them adds 1024 bytes to the
  executable on disk and one adds none. Check with `size` — the difference is
  visible in the numbers, and it is the README probe at scale.
- **9 vs 10.** `const char *india` and `const char juliet[]` look like the same
  thing and are not. One is a *pointer* that happens to be const-qualified at
  the far end; the other *is* the characters. They land in different sections
  and one of them involves a relocation. Draw both in memory.
- **11 and 12.** Initialised to `NULL`, which is all zero bits — so despite
  having an initialiser they behave like case 3.
- **14 vs 15.** Both are declared inside a function. One has a fixed address
  for the life of the program and a (mangled) symbol; the other has neither.
  `static` on a local changes its *storage duration*, not its scope. `static`
  on a global changes its *linkage*, not its storage. Same keyword, two
  unrelated jobs — which is the single most confusing thing about it.

## After revealing

- [ ] Every prediction correct, or you can say why the wrong ones were wrong
- [ ] `size sections.o` — account for the `data` number by hand, byte by byte.
      It should come out to `alpha` + `echo_` + `hotel` + `india` + ... work
      out which of the twelve are in it and add up their sizes.
- [ ] Change `char golf[1024];` to `char golf[1024] = {1};` and re-run
      `./reveal.sh`. One byte of initialiser moves 1024 bytes from `.bss` to
      `.data` and grows the file. Explain that to yourself out loud.
