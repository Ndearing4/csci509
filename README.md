# CSCI 509 prep — Aug 10 → Sept 23, 2026

Getting from "never written C" to "can write and debug C in a UNIX environment"
before **CSCI 509 Operating Systems Internals** (Dr. Tarek Idriss) starts on
**Sept 23**. Budget ~10–12 hrs/week, ~70 hours total.

Self-paced. Each module has a **skip-ahead probe** (answer it instantly → skip the
module) and an **exit test** (a thing you can *do*, not a list you've read). Tick
boxes as exit tests pass.

Full plan: `/mnt/c/Users/natha/.claude/plans/okay-so-i-need-encapsulated-gadget.md`
Course research: `reference/course-findings.md`

**This repo lives in the WSL filesystem at `~/csci509`** — real ext4, not `/mnt/c`.
That's for parity with the department Linux machines, and it keeps line endings,
exec bits, and `make` timestamps honest. From Windows it's reachable at
`\\wsl$\Ubuntu\home\nathandearing4sure\csci509`, but **edit it from inside WSL**.

---

## Do this first

- [x] **Email Dr. Idriss** — sent, no reply, **written off 2026-08-15.** The two
      conditional modules were decided without him: M5 dropped, M4b demoted to
      a stretch goal. See "Decided without Idriss" below.
- [x] **Run the toolchain setup** — `cd setup && ./setup-wsl.sh`
- [x] **CSCI 247 coverage** — have a Fall 2025 lecture log, see
      `reference/csci247-topic-log.md`. Added module M8.
- [x] **CSCI 347 coverage** — Winter 2025 syllabus. Chapter map baked into M3/M6.
- [ ] Confirm Idriss and the Sept 23 start on the Fall 2026 listing.

---

## Progress

| | Module | Hours | Priority | Done |
|---|---|---|---|---|
| M0 | Toolchain | 1–2 | Critical | [ ] |
| M1a | C syntax foundations | 1–2 | Critical | [ ] |
| M1 | C data model | 6–10 | Critical | [ ] |
| M2 | C memory & tooling | 8–14 | Critical | [ ] |
| M3 | UNIX systems programming | 10–16 | Critical | [ ] |
| M4a | Linking & compilation pipeline | 4–6 | Important | [ ] |
| M6 | Concurrency | 8–12 | Important | [ ] |
| M8 | Memory hierarchy & virtual memory | 5–7 | Important | [ ] |
| M7 | Concept reading | ~10 | parallel | [ ] |
| M4b | Bare-metal RISC-V kernel | 10–16 | **stretch** | [ ] |
| ~~M5~~ | ~~C++ subset~~ | — | **dropped** | — |

The committed set is **53–79h** against a ~70h budget, so it no longer fits
cleanly. M8 came out of the 247 log and earns its place — "memory management" is
the *first* item in 447's catalog scope and the plan had nothing on it. **If time
runs short, cut M7's reading breadth, not M8's.** Skimming Silberschatz is the
most recoverable thing here; it's the one part of this plan that lecture will
cover again anyway.

### Decided without Idriss (2026-08-15)

No reply, and none expected. Both conditionals resolved on the merits:

**M5 (C++) — dropped.** It was conditional on confirmation that never came, and
most OS courses are C. Cheap to add back if the syllabus says otherwise; the
syllabus posts before Sept 23 either way. Spending 5–8h on it speculatively is
the worse bet.

**M4b (RISC-V) — kept, but as a stretch goal, not committed.** Honest arithmetic:
at 10–12 hrs/week from Aug 15 there are roughly **55–66h left** before Sept 23.
The committed core alone is 52–77h. Adding M4b makes it **62–93h**, which
overflows the budget outright. So M4b is explicitly gated: *start it only if the
core is done and you're ahead at ~Sept 1.* An unconfirmed kernel project is a
worse use of the last 10–16h than finishing M3 and M6 properly, and those are
needed under every instructor.

M4b also can't run on the department machines — it needs QEMU and a cross
compiler, and there's no root there. It stays a WSL job.

**Order:** M0 → M1a → M1 → M2 → M3 → M4a → M8 → M6, with M7 alongside from day one.
**Depends on:** `M0 → M1a → M1 → M2 → {M3, M4a}`, `M3 → M6`, `M2 → M8`, `M4a → M4b`.

---

## M0 — Toolchain · 1–2h · `setup/`

- [x] **Toolchain installed and verified.** gcc/g++ 11.4.0, GNU Make 4.3, gdb 12.1,
      valgrind 3.18.1, binutils 2.38 (`objdump`/`readelf`/`nm`), and `man 2` works.
- [x] **Workspace moved to WSL native ext4** at `~/csci509`. Every file confirmed
      LF-only; `setup-wsl.sh` still executable and passes `bash -n`.
- [x] **Parity program builds clean in WSL** under `-std=c11 -Wall -Wextra -Werror`.
      Source in `setup/parity/`, instructions in `setup/parity/BUILD.md`.
- [ ] **Exit test — the remaining half:** build and run it on a department Linux
      box over ssh and diff the output. The department machine is the grading
      environment, so parity there is the check that matters. `BUILD.md` has the
      `scp`/`ssh` one-liner and a table of what each possible mismatch means.

**Access note (2026-08-15).** `ssh cf405-07` works from a plain Windows terminal
on the campus VPN. It does *not* work from inside WSL: with the OpenVPN adapter
at interface metric 1, WSL2's NAT loses both DNS and the route to campus, and
Windows 10 has no mirrored-networking mode to fix it. The VPN is also flaky
enough that chasing this is not worth it. **Nothing downstream is blocked** — M1
through M8 need nothing but gcc, which works locally. Do the parity diff by hand
in a Windows terminal whenever the connection is behaving.

### This machine's fingerprint (WSL, 2026-08-11)

Recorded so the department diff has something to compare against:

| | |
|---|---|
| gcc | 11.4.0, `__STDC_VERSION__` 201112L under `-std=c11` |
| model | LP64 — `int` 4, `long` 8, `void *` 8, `size_t` 8, `long double` 16 |
| `char` | **signed**, 8 bits |
| byte order | **little endian** — `0x01020304` stores as `04 03 02 01` |
| layout | literal < static < heap ≪ stack, ASLR on |

If the department box is also x86-64 Linux, all of this should match. A mismatch on
`char` signedness or byte order means it isn't x86 — worth knowing before writing
code that assumes otherwise.

## M1a — C syntax foundations · 1–2h · `m1-data/c-syntax/`

**Added 2026-08-17, mid-M1.** CS:APP is not a C tutorial. It teaches machine
representation and assumes you already write C fluently. §2.1 lands endianness
cleanly and then hands you a code block written in syntax it never taught. That
gap is in the material, not in you, and re-reading §2.1 does not close it.

`c-syntax/tour.c` covers the eight constructs the rest of M1 assumes, each
demonstrated on a problem that is deliberately *not* `show_bytes`:

1. `printf` — the first argument is always a format string; width and zero-pad flags
2. `&`, the address-of operator
3. Declaring a pointer; `*` in a declaration vs `*` in an expression
4. Pointer arithmetic scaling by the pointed-to type
5. `a[i]` is *defined* as `*(a + i)` — indexing is not a separate feature
6. `for` loops, and why the counter is `size_t` rather than `int`
7. Casting a pointer — same address, different read width
8. Passing pointers so a function can modify its caller's variable

Run it, then break it. Comments in sections 1, 2, 6 and 7 say what to change.
Section 6 especially: swap `size_t i` for `int i`, rebuild, and read the
`-Wsign-compare` error — that is the exact error CS:APP's `show_bytes` produces
under these flags.

- [ ] **Exit test:** predict the output of all eight sections before running
      them, and explain why `int *p; p + 1` advances four bytes while a
      one-byte pointer advances one.

**Reading, if the tour is not enough:** Beej's Guide to C Programming (free,
written for exactly this gap), K&R chapter 5, or Modern C levels 0–1. One of
the three, not all three.

## M1 — C's data model · 6–10h · `m1-data/`

**Skip-ahead probe:** without running it, what does `printf("%zu\n", sizeof(arr))`
print inside a function taking `int arr[10]`, versus in the scope where `arr` was
declared? Why?

Integer types, signed vs unsigned, two's complement, overflow, integer promotion.
Bit manipulation: masks, shifts, `& | ^ ~`. Endianness. Hex fluency. Arrays,
pointer arithmetic, array/pointer decay, pointers to pointers with `argv`.

One hour, no more, on **IEEE 754 float representation** — sign/exponent/mantissa,
why `0.1 + 0.2 != 0.3`. Heavy in 247 (W3) and on its exams, but thin on 509
relevance. Enough to not be surprised by it.

- [ ] **Exit test:** datalab-style bit puzzles solved with no loops or
      conditionals, plus a function printing any object's bytes in memory order.

**Scaffolding is built and ready — see `m1-data/README.md`.** Twelve puzzle stubs
in `m1-data/bits/` with a brute-force harness (36 edge values + 200k deterministic
random cases per puzzle) and a `check-rules.sh` that catches banned constructs;
`m1-data/bytes/` has the `show_bytes` stub and a nine-section driver covering
endianness, two's complement, IEEE 754, struct padding, and array decay. Both
build clean under `-Wall -Wextra -Werror` with the stubs in place. **Start here.**

## M2 — C memory and tooling · 8–14h · `m2-memory/`

**Skip-ahead probe:** write a linked-list insert taking `Node **head`. Why the
double pointer?

Stack vs heap vs static. `malloc`/`free`, dangling pointers, use-after-free,
double-free. `struct`, `typedef`, structs holding pointers, a linked list built by
hand. C strings: null termination, `strcpy` vs `strncpy`, off-by-one.

The tooling half transfers to everything downstream: **gdb** (breakpoints,
`step`/`next`, `print`, `backtrace`, `x/`), **valgrind**, and **make** written by
hand. Header guards, `.h`/`.c` separation, `extern`.

- [ ] **Exit test:** a linked list split across header and implementation with a
      hand-written Makefile. Introduce a use-after-free deliberately and find it
      in gdb *before* confirming with valgrind.

*Stretch, revisit after M8:* 247 has students **write their own `malloc`** on top
of `sbrk` (W8). It's the best single exercise for understanding what the heap
actually is, and it's the closest thing in either prerequisite to real OS memory
management. Skip it on the first pass through M2; come back if M8 lands early.

## M3 — UNIX systems programming · 10–16h · `m3-unix/`

The 347 gap, and the assumed background whoever teaches. APUE Ch 1, 3, 4, 5, 6, 7,
8, 9, 10, 11, 12, 15.2–15.3.

`man 2` vs `man 3` — the syscall/library boundary is exam material. File
descriptors, `open`/`read`/`write`/`close`/`lseek`, `errno`. The stdio layer on top
(`FILE*`, buffering, why it differs from raw fds). `stat`, `opendir`/`readdir`.
`fork`, `exec*`, `wait`/`waitpid`, exit statuses, orphans and zombies. `pipe` and
`dup2`. Signals via `sigaction`. Skim shared memory and message queues.

**Project: a small shell** — prompt, parse into argv, fork/exec, wait, `|`, `<`,
`>`, Ctrl-C.

- [ ] **Exit test:** `ls -la | grep foo > out.txt` works; Ctrl-C kills the child,
      not the shell; `ps` shows no zombies after a hundred commands.

## M4a — Linking and the compilation pipeline · 4–6h · `m4a-linking/`

Preprocess → compile → assemble → link. Object files, symbols, relocation, static
vs dynamic linking, ELF sections. `objdump`, `nm`, `readelf`.

**Plus ~2h of reading assembly, not writing it.** 247 spends three weeks (W3–W5)
on machine organization and its final project is an assembler for a toy 16-bit
ISA — that whole track is aimed at 247's own capstone and isn't worth chasing.
What *is* worth having: `objdump -d` your own C and recognize the prologue and
epilogue, where locals live, how arguments get passed, what the frame pointer is
doing. That's the skill that pays off the first time gdb stops somewhere with no
source.

- [ ] **Exit test:** explain a real multi-file link error from the symbol table,
      predict which section a given variable lands in before checking with
      `readelf`, and walk through the disassembly of a two-argument function of
      your own naming what each instruction does to the stack.

## M6 — Concurrency · 8–12h · `m6-concurrency/`

347's most strongly worded outcome, and "concurrent process management" sits in
447's catalog scope. Safe bet whoever teaches. APUE Ch 11–12.

pthreads create/join, mutexes, condition variables. The classical problems:
bounded buffer / producer-consumer, readers-writers, dining philosophers. The four
conditions for deadlock. Monitors and rendezvous specifically.

- [ ] **Exit test:** producer-consumer and readers-writers stay correct under
      stress; state the four deadlock conditions cold; say what a monitor gives
      you that a bare semaphore doesn't.

## M8 — Memory hierarchy & virtual memory · 5–7h · `m8-memory/`

**Skip-ahead probe:** a program sums a large 2-D array. Why is row-major traversal
several times faster than column-major, when both touch exactly the same values?

Added after reading the 247 log, which spends **two full weeks** here (W6–W7)
plus a third on heap internals (W8). The plan previously had none of it, and
"memory management" and "memory protection" are the first and last items in 447's
catalog scope — so this is the highest-value thing the 247 material surfaced.

Cache basics: lines, associativity, hit/miss, eviction. Locality and why loop
order changes runtime. Then the part 509 actually builds on: **virtual memory** —
address translation, page tables and multi-level page tables, the TLB, page
faults, what "memory protection" means mechanically. Finally `sbrk`/`mmap` and
what `malloc` really asks the kernel for, and what `free` does *not* give back.

Read **Silberschatz Ch 9–10** here rather than in M7 — this is the one stretch of
the book worth reading properly instead of skimming.

- [ ] **Exit test:** measure the row-major vs column-major difference on your own
      machine and explain the number. Translate a virtual address to a physical
      one through a two-level page table by hand. Say what happens, step by step,
      from the instruction that touches an unmapped page to the one that retries.

## M7 — Concept reading · ~10h · `notes/` · parallel

An hour or two a week alongside the coding, from the start. Spaced beats blocked.

**Silberschatz Ch 1–8**, skimming for vocabulary rather than mastery. Safe spine:
standard text across WWU's 447 sections, matches the catalog scope. **Ch 9–10 are
deliberately not here** — they're memory management, and they get read properly in
M8 instead of skimmed.

Nelson's public slides are a *different instructor's* emphasis — usable as a free
sanity check on scope, but not authoritative. Once Idriss posts his own slides
they supersede everything, since reviews are unanimous that his exams track them.

- [ ] Ch 1–2 · [ ] Ch 3–4 · [ ] Ch 5 · [ ] Ch 6–7 · [ ] Ch 8

---

## Stretch — only if ahead of schedule at ~Sept 1

### M4b — Bare-metal RISC-V kernel · 10–16h · `m4b-riscv/`

No confirmation from Idriss, so this is a bet on xv6-riscv being the modern
default. Worth making *only* with the committed core finished — see the
arithmetic under "Decided without Idriss". Needs M4a, and
`./setup-wsl.sh --with-riscv` (not yet run; RISC-V packages are not installed).
WSL only — the department machines have no QEMU and no root to install it.

RISC-V registers, calling convention, reading disassembly, the three privilege
modes. QEMU's `virt` boot model. Linker script, assembly entry stub that sets up a
stack and calls into C, `putchar` writing to the UART's MMIO address, then
character input.

- [ ] **Exit test:** boots under `qemu-system-riscv64 -machine virt -nographic`,
      echoes typed characters, and `gdb-multiarch` on QEMU's gdbstub (`-s -S`)
      breaks in the C entry point.

**No answer by ~Sept 1? Do it anyway.** xv6-riscv is the most likely modern
default, and the skills transfer to any kernel project. Build from public
QEMU/RISC-V docs only.

### ~~M5 — C++ subset~~ · dropped 2026-08-15

Dropped rather than deferred: no confirmation came, most OS courses are C, and
the syllabus will settle it before the quarter starts. If it *does* turn out to
be C++, the recovery is 5–8h on classes across header/implementation files,
public/private, inheritance, `inline`, constructors, namespaces, and reference
parameters — actively skipping templates, STL, and iostream. The `m5-cpp/`
directory stays empty as a placeholder.

---

## Ground rules

**Academic honesty.** Idriss's AI policy is unknown. Nelson's — the other 447
instructor — permits AI for understanding concepts and generating practice
questions, and forbids uploading assignment descriptions or asking for assignment
code. Assume that until Idriss says otherwise: it's the stricter reading and the
cost of guessing wrong is asymmetric. Everything in this repo is self-chosen
exercises, so none of it is affected. Never seek out past students' solutions for
any WWU course — instructors here compare submissions against prior years.

**Environment.** All C written and compiled *inside WSL*, UNIX line endings, never
edited Windows-side. Editor and command line, no IDE — both 347 and 447 ban them,
so build the habit now.

**Code hygiene.** Nelson's coding standard isn't binding here, but its content is
generically good C practice and costs nothing as a default: file header comment
with author and date, consistent indentation, header guards in every header,
descriptive names, sparing globals, comments before functions and loops, a short
comment on each variable declaration.

**Every module builds clean under `-Wall -Wextra -Werror`, and anything touching
the heap is valgrind-clean.**
