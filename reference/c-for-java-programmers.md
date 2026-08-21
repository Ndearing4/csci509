# C, for someone who has written Java

Written 2026-08-21, alongside the M2–M4a scaffolding.

A translation reference, not a tutorial. The three `tour.c` files teach by
running; this is the thing to skim first and come back to when something
looks arbitrary.

**The one-sentence version:** Java gives you objects, references, a garbage
collector, exceptions, and bounds checking. C gives you bytes, addresses, and
`free`. Almost every difference below follows from that one.

Nothing here is new material — it is the same facts the tours demonstrate,
arranged by "what did I call this in Java".

---

## The three tours, in order

| | | teaches |
|---|---|---|
| `m1-data/c-syntax/tour.c` | M1a | printf, `&`, pointers, `a[i]` is `*(a+i)`, casts |
| `m2-memory/c-structs/tour.c` | M2 | struct, `->`, malloc/free, `Node **`, strings, storage |
| `m3-unix/syscalls/fdtour.c` | M3 | file descriptors, read/write/lseek, errno, dup2, stdio |

All three are finished programs. Run them, then break them.

---

## Memory

The big one, and the source of most of the rest.

| Java | C |
|---|---|
| `new Foo()` allocates and zeroes | `malloc(sizeof *p)` allocates; contents are **garbage** |
| the GC frees it, eventually | you call `free(p)`, exactly once, or it leaks |
| unreachable means collected | unreachable means **leaked** |
| a stale reference cannot exist | a stale pointer is normal and reads fine right up until it does not |
| `null` dereference throws | a null dereference is a segfault, and a *wild* pointer may not even crash |

Three lifetimes instead of two, and you pick:

```
stack    a local. Dies at the closing brace.        automatic
heap     malloc'd. Dies when you free it.           yours
static   a global, or a `static` local. Dies at exit. automatic
```

Every bug in `m2-memory/bugs/` is a pointer used outside one of those three
windows. That is the definition of the category, not a coincidence.

**`calloc` zeroes; `malloc` does not.** Java's zero-initialisation of fields
is a guarantee you no longer have.

---

## struct is not class

| Java | C |
|---|---|
| `class Point { int x, y; }` | `struct Point { int x; int y; };` |
| fields, methods, access control | fields. That is all. |
| `Point b = a;` — two names, **one object** | `struct Point b = a;` — a **copy**, 8 bytes memcpy'd |
| `a.equals(b)` | compare fields by hand; `==` on structs is a compile error |
| `item.restock(5)` | `restock(&item, 5)` |
| `this` | the first parameter, passed explicitly |
| `toString()` | write a print function, or a `printf` per field |
| `null` | `NULL` |

There is no inheritance, no interfaces, no generics, no overloading — two
functions cannot share a name, which is why C library names look like
`list_push_front` rather than `push`. The prefix *is* the namespace.

---

## Pointers vs references — where Java misleads you most

A Java reference and a C pointer are close relatives. The difference that
matters: **in C you can take the address of a variable, including of a
pointer variable.** Java has no `&`.

Both languages pass **everything by value.** Java passes references by value.
So this Java method does nothing useful:

```java
void selectFirst(Item sel) { sel = catalogue[0]; }   // assigns to the copy
```

and neither does its C twin:

```c
void select_first(Item *sel) { sel = &catalogue[0]; }  /* same mistake */
```

Java's fixes are to return the new value or set a field. C has a third
option — take the address of the caller's *pointer*:

```c
void select_first(Item **sel) { *sel = &catalogue[0]; }
```

That is the whole derivation of `Node **head`, and there is nothing else to
it. `m2-memory/c-structs/tour.c` section 5 runs both versions side by side.

Read declarations right to left: `Item **sel` is *sel is a pointer, to a
pointer, to an Item*.

---

## Arrays and strings

| Java | C |
|---|---|
| `arr.length` | nothing. You keep the count yourself. |
| `arr[10]` throws `ArrayIndexOutOfBoundsException` | `arr[10]` reads whatever is there, silently |
| `String` is a class, immutable, knows its length | a `char *` to bytes ending in `'\0'`. That is the entire type. |
| `s.length()` | `strlen(s)` — **walks the bytes**, O(n), every call |
| `s1.equals(s2)` | `strcmp(s1, s2) == 0` |
| `s1 == s2` compares references | `s1 == s2` compares addresses. Same trap, same fix. |
| `s1 + s2` | `strcat`, into a buffer you sized yourself |
| `new int[n]` | `malloc(n * sizeof(int))` |

The rule that generates every string bug: **a buffer of N bytes holds N-1
characters.** The terminator is not free.

`sizeof arr` is the array's size in bytes and `strlen(s)` is the string's
length in characters, and they answer different questions — one at compile
time from the type, one at run time from the bytes. In a function taking
`int arr[10]`, `sizeof arr` is the size of a *pointer*, because the array
decayed. That is M1's skip-ahead probe.

---

## Errors

There are no exceptions. Nothing is thrown, nothing unwinds, and nothing
forces you to handle anything.

| Java | C |
|---|---|
| `throw new IOException(...)` | return -1 (or NULL), and set `errno` |
| `catch (IOException e)` | `if (rc == -1) { ... }` |
| `e.getMessage()` | `strerror(errno)`, or `perror("what")` |
| checked exceptions force handling | nothing does. An unchecked return is silent. |
| `finally` / try-with-resources | call `close`/`free` yourself, on **every** path |

Three rules for `errno`:

1. Check the **return value** first. `errno` is only meaningful after a call
   that failed — it is not cleared on success.
2. Read it **immediately**. Any library call in between, `printf` included,
   may overwrite it.
3. Look up what the values mean in the man page's ERRORS section, not by
   guessing.

The absence of `finally` is why C code has more `goto cleanup;` than you
would expect, and why that is idiomatic rather than a code smell.

---

## The compilation model

Java compiles per class, resolves names at load time, and `import` finds
things for you. C does none of that.

**The compiler sees one `.c` file at a time and has no idea any other file
exists.** A `.h` file is not an import — it is a text file that gets pasted
in by the preprocessor before compilation starts.

| Java | C |
|---|---|
| `import com.foo.Bar;` | `#include "bar.h"` — literal text substitution |
| the class file has everything | declaration in the `.h`, definition in the `.c` |
| one compilation step | preprocess → compile → assemble → **link** |
| `NoClassDefFoundError` at run time | `undefined reference to 'foo'` at link time |
| `public` / `private` | nothing / `static` at file scope |

**Declaration vs definition** is a distinction Java does not make and C
depends on:

```c
int add(int a, int b);          /* declaration: it exists somewhere */
int add(int a, int b) { ... }   /* definition:  and here it is */
extern int counter;             /* declaration of a variable */
int counter = 0;                /* definition */
```

A header may be included by fifty files, so it must contain **declarations
only**. Put a definition in one and you get `multiple definition` — which is
`m4a-linking/symbols/break-it.sh` case 2.

`static` means two unrelated things, and this confuses everyone:

- on a **global or function**: internal linkage — invisible to other files.
  This is `private`.
- on a **local variable**: static storage duration — one instance, kept
  between calls. This is closer to a `private static` field.

M4a is entirely about this layer. If `undefined reference` looks like a
compiler error to you, that module is the fix.

---

## Syntax you will trip on

| | |
|---|---|
| `#include <stdio.h>` vs `"myheader.h"` | angle brackets: system paths. Quotes: here first. |
| `%d %s %zu %p %f %c %x` | printf has no overloading, so you name the type. A wrong one is undefined behavior, not a wrong-looking string. |
| `for (size_t i = 0; i < n; i++)` | `size_t` because it is unsigned, and comparing it against a signed `int` warns under `-Wextra`. |
| `if (x = 1)` | assigns, then tests. Legal C, and a bug. gcc warns; write `if (1 == x)` if you like belts. |
| no `boolean` | `0` is false, everything else is true. `<stdbool.h>` gives you `bool`, and this repo mostly uses `int`. |
| `char` is a number | 8 bits, and **signed** on x86-64. `strcmp` compares as `unsigned char` for exactly that reason. |
| integer division truncates | as in Java. But signed overflow is *undefined*, not wraparound — the compiler may assume it never happens. |
| `//` comments | fine in C99+. `/* */` does not nest. A `*/` inside a comment ends it, including one hiding inside a regex. |

---

## Things with no Java equivalent at all

- **The preprocessor.** `#define`, `#include`, `#ifdef` — a text
  substitution pass that runs before the compiler and does not understand C.
  A macro cannot be inspected in a debugger because by then it does not
  exist.
- **Pointer arithmetic.** `p + 1` advances by `sizeof *p`, not one byte.
- **`sizeof`.** Compile-time, from the type. `sizeof *p` is preferred over
  `sizeof(Type)` because it stays right when the type changes.
- **Undefined behavior as a language concept.** Not "unspecified result" —
  the compiler is permitted to assume it never happens and optimise on that
  basis. Signed overflow, reading uninitialised memory, and shifting by 32
  are all in this category, and all three appear in this repo's exercises.
- **Manual `const`.** A promise to the compiler, and documentation to the
  reader. `final` is the nearest thing and is not close.
- **Header guards.** `#ifndef FOO_H` — because text substitution has no idea
  it has already pasted this file in once.

## Things in Java with no C equivalent

Generics, interfaces, inheritance, overloading, packages, reflection,
`toString`, `equals`/`hashCode`, autoboxing, `ArrayList`, `HashMap`,
`String`, exceptions, `finally`, garbage collection, bounds checking,
threads in the language itself (C has pthreads, a library — M6).

C's standard library is tiny and there is no collections framework. If you
want a hash map you write one, or you link a library someone else wrote.
That is why "write a linked list" is a real exercise here and was a toy one
in your data structures class.

---

## Ranked by how much time it will cost you

1. **Forgetting `free`, or freeing twice.** No GC. `m2-memory/bugs/`.
2. **A buffer one byte too small.** The terminator. `m2-memory/strings/`.
3. **Assuming a struct assignment aliases.** It copies.
4. **Not checking a return value**, then reading a stale `errno`.
5. **`undefined reference`** and not knowing it came from the linker. M4a.
6. **Reading uninitialised memory** and getting a plausible answer.
7. **`sizeof` on an array parameter**, which is a pointer. M1's probe.
8. **A dangling pointer that works**, until an unrelated change moves the
   allocation.

Six of those eight produce a program that *runs and looks correct*. That is
the real adjustment from Java: the compiler and the runtime have stopped
telling you when you are wrong, and `-Wall -Wextra -Werror`, gdb, and
valgrind are what you have instead. That is why every module in this repo
builds under those flags and why M2 spends a third of itself on tooling.
