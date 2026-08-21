# Four stages, and what to look for in each

```sh
./pipeline.sh
```

It leaves everything in `artifacts/`. Answer these from the files, not from
the script's commentary — the commentary is there to check yourself against
after you have looked.

## 1. Preprocess — `artifacts/demo.i`

- [ ] `demo.c` is 65 lines and `demo.i` is around 700. Where did the rest come
      from, and what would `#include <stdlib.h>` as well do to that number?
- [ ] Find the line that used to say `char message[32] = GREETING;`. What does
      it say now? Is `GREETING` anywhere in the file?
- [ ] Find where `TWICE(5)` went. There is no `TWICE` in `demo.i`.
- [ ] `grep -c '^#' demo.i` — those are line markers, telling the compiler
      which original file and line each chunk came from. That is how an error
      inside a header reports the header's name.
- [ ] **Given all that: why can gdb not print the value of a macro, and why
      does `TWICE(i++)` increment `i` twice?** Both answers are the same
      sentence.

## 2. Compile — `artifacts/demo.s`

This is the only stage that is actually *compiling*. C in, assembly out.

- [ ] Find the `.data`, `.bss` and `.rodata` directives. For each of the seven
      globals in `demo.c`, find which one it landed under.
- [ ] `initialised_global` is under `.data` and its value 7 is written out
      literally. `zero_global` is under `.bss` and there is no 0 anywhere.
      **Why does the zero not need storing?**
- [ ] `.globl add` is there. There is no directive at all for `triple`.
      What does `static` do, in terms of what the assembler is told?
- [ ] `call_count` shows up as `call_count.0` with `.local`. A `static` local
      is not on the stack — it has a fixed address like a global — but its
      name is mangled so two files can each have one. Find its section.
- [ ] `literal` is a `const char *`. The pointer and the string it points to
      are in *different* sections. Find both.

## 3. Assemble — `artifacts/demo.o`

- [ ] `nm demo.o`. Go through every line and say why that symbol has that
      letter. The uppercase/lowercase distinction is global vs local.
- [ ] Three symbols are `U`. Name them, and say which file or library each one
      is going to be found in.
- [ ] `private_global` is `d` and `initialised_global` is `D`. Same section,
      different case. What can the linker do with one and not the other?
- [ ] `objdump -r demo.o` — the relocation list. Find the entry for
      `shared_counter_bump`. `R_X86_64_PLT32` is "a 32-bit PC-relative call
      through the procedure linkage table". You do not need the details; you
      need to see that the call instruction currently has a **hole** in it.
- [ ] `objdump -d demo.o`, find the `call` to `shared_counter_bump`, and look
      at the four bytes of the operand. Then do the same on the linked
      `artifacts/demo` and compare. That difference is what linking *is*.

## 4. Link — `artifacts/demo`

- [ ] `size demo.o counter.o demo`. The executable's text is bigger than the
      sum of the two objects. What did the linker add?
- [ ] `nm demo | grep shared_counter_bump` — `U` became `T`. Where is it now?
- [ ] `nm -u demo` still lists `printf` and `strlen`. **The link succeeded and
      they are still undefined.** Who resolves them, and when?
- [ ] `ldd demo`. Three entries. One of them is not a file on disk
      (`linux-vdso.so.1`) — look up what the vDSO is, two minutes, it is the
      kernel mapping a page of code into every process so that things like
      `gettimeofday` do not need a real system call.
- [ ] `readelf -d demo | head` — the `NEEDED` entries are the same list from a
      different angle.

## The summary worth writing down

In one sentence each:

1. The preprocessor is ______ , and does not understand C.
2. The compiler works on ______ at a time, so a call to a function in another
   file becomes ______ plus ______ .
3. The linker's two jobs are ______ and ______ , and its two error messages
   are the two ways each can fail.
4. Linking is not the last step, because ______ .
