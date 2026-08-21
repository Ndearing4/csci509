# M4a — Linking and the compilation pipeline · 4–6h

**Skip-ahead probe:** a global `int counter = 0;` and a global `int total = 7;`
are both 4 bytes. One of them makes the executable 4 bytes bigger and one does
not. Which, and why?

Instant answer → do `disasm/` only. Otherwise start at `pipeline/`.

This module is short and it is mostly *reading output*, not writing code.
Nothing here is a program you build from scratch; the exercises are scripts
that produce artifacts and worksheets asking you to predict what will be in
them before you look.

## The three exercises

```
pipeline/   preprocess -> compile -> assemble -> link, one stage at a time
symbols/    object files, symbol tables, real link errors, static vs shared
disasm/     read the assembly gcc produced for code you wrote
```

Two to three hours for the first two, one to two for `disasm/`.

## What this is for

Every error message in the second half of this list is one you will otherwise
lose an hour to:

```
undefined reference to `foo'
multiple definition of `counter'
relocation R_X86_64_PC32 against symbol `bar' can not be used when making
  a shared object; recompile with -fPIC
error while loading shared libraries: libfoo.so: cannot open shared object file
```

None of those come from the compiler. All four come from the *linker* or the
*loader*, after every file compiled fine, and they are about the program as a
whole rather than about any one file. That distinction is the module.

## The 247 track this deliberately does not chase

247 spends three weeks on machine organization and ends with an assembler for
a toy 16-bit ISA. That is aimed at 247's own capstone, and writing an
assembler is not on the path to 509.

What *is* worth having, and is what `disasm/` covers: `objdump -d` your own C,
recognise the prologue and epilogue, see where locals live, follow how
arguments get passed, know what the frame pointer is doing. That is the skill
that pays off the first time gdb stops somewhere with no source — which in an
OS course is most of the time.

**About two hours of reading assembly, not writing it.** If you find yourself
writing `.s` files by hand, you have wandered off.

## Exit test

- [ ] Explain a real multi-file link error from the symbol table — `symbols/`
      has five, and `nm` output for each, and the answer is derivable from it
- [ ] Predict which section each of the twelve declarations in
      `symbols/sections.c` lands in, on paper, before `./reveal.sh` shows you
- [ ] Walk through the disassembly of your own two-argument function naming
      what every instruction does to the stack — `disasm/ANNOTATE.md`
- [ ] Say what `-fPIC` is for and why a static library does not need it

## The tools, and what each is for

| | |
|---|---|
| `gcc -E` | stop after preprocessing. Text in, text out. |
| `gcc -S` | stop after compiling. C in, assembly out. |
| `gcc -c` | stop after assembling. One `.o`, not linked. |
| `nm file.o` | the symbol table. **T**ext, **D**ata, **B**ss, **U**ndefined. |
| `nm -D lib.so` | the *dynamic* symbol table — what a shared library exports. |
| `nm -C` | demangle C++ names. Not needed here, needed the day you meet C++. |
| `readelf -S` | section headers: names, sizes, addresses. |
| `readelf -s` | symbol table, with more detail than `nm`. |
| `readelf -d` | dynamic section — which libraries this binary needs. |
| `objdump -d` | disassemble the text section. |
| `objdump -r` | relocations: the holes the linker has to fill in. |
| `ldd prog` | which shared libraries get loaded, and from where. |
| `size prog` | text/data/bss totals. The fastest answer to the probe above. |

`nm` and `readelf -s` overlap. `nm` is quicker to read, `readelf` tells you
more; learn `nm`'s one-letter codes first because they are what people quote
at each other.

## The one-paragraph version

The compiler works on **one translation unit at a time** and has no idea any
other file exists. When it sees a call to a function defined elsewhere it
emits the call with a hole where the address goes, and a **relocation** saying
"put the address of `foo` here". The linker collects every `.o`, lays out the
sections, resolves each undefined symbol against some other file's definition,
fills in the holes, and complains if a symbol has no definition (undefined
reference) or more than one (multiple definition).

Everything in `symbols/` is a consequence of those two failure cases.
