# CSCI 247 — what was actually covered

Source: another student's day-by-day lecture log, Fall 2025 section. Topic list
only — no assignments, no code. More useful than a syllabus, because it records
what got covered and how long each thing took rather than what was planned.

## Course shape

- Attendance/lab 10%, homework 45%, midterm 10%, final project 15%, final 20%
- Weekly homework, **autograded**, no late days
- Midterm (W5) and final both **open book**
- Final project: **a computer systems simulator and an assembler** for a 16-bit
  toy ISA ("x16")
- Instructor unidentified from the log; not evidently Nelson

## Week-by-week

| Week | Topics |
|---|---|
| W1 | Intro to C. Why systems: hardware → ISA → OS → syscall interface → high-level languages. Hex and binary. Pointers. Reading input. Signed/unsigned. |
| W2 | Number representation. Sign extension, storage schemes, bit patterns. Network byte order. Bitwise operators. |
| W3 | Representing real numbers / floating point. How computers are built: semiconductors, transistors, logic gates. Hardware control language. |
| W4 | Assembly code. Registers. The stack. The instruction cycle. Cycles per instruction. ARM vs CISC. |
| W5 | Pipeline hazards — branch and memory. Boolean algebra. **`malloc`/`free`. Linked lists in C.** Midterm. |
| W6 | Optimizing loops. **Caches** — including eviction policy. |
| W7 | Hash tables and cache. **Memory. Paging. Virtual memory.** Requesting more memory. |
| W8 | Units of measure. **`sbrk` and `malloc`. ELF files.** Wrote a malloc implementation in class. |
| W9 | **Computer boot process.** Real space usage and **system calls**. **`fork`.** |
| W10 | **Threads and parallel processing.** Man page exercise. Course recap. |

## What this changes

Bolded rows above are the ones that touch CSCI 509's catalog scope directly
(memory management, system call implementation, memory protection).

**Two full weeks on the memory hierarchy and virtual memory** (W6–W7), plus a
third on `sbrk`/`malloc` internals and ELF (W8). This was the single largest gap
in the prep plan — it had no coverage at all. Module **M8** now exists for it.

**Assembly and machine organization run W3–W5** — three weeks, and the final
project is an assembler. The plan does not cover this and mostly still shouldn't:
building an assembler is 247's capstone, not 509 preparation. What *is* worth
taking is the ability to read `objdump -d` output of your own C and recognize the
stack frame and calling convention. Folded into M4a as ~2h, not a new module.

**Floating-point representation** (W3) is 247 exam material with little 509
relevance. One hour inside M1, no more.

**`malloc`/`free`, linked lists, `fork`, threads, system calls, ELF** all appear
here *and* in 347. Both prerequisite courses cover them, which is strong evidence
that 509 assumes them cold. Confirms M2, M3, M4a, and M6 as correctly scoped —
these were already the plan's Critical modules.

**The boot process** (W9) is the one topic that ties to the conditional M4b. If a
kernel project is confirmed, this is prior context Nathan won't have.
