# CSCI 509 — what's known about the course

Research record as of 2026-08-10. Sources at the bottom. Keep the
confirmed/unconfirmed split intact when updating — conflating the two is what
sent the first draft of the plan wrong.

## Confirmed, instructor-independent

- **CSCI 509 "Operating Systems Internals" is the graduate substitute for
  CSCI 447 "Operating Systems"**, and is one of four MS core courses (students
  take three of the four).
- **Catalog scope for 447:** "This course explores the services operating systems
  provide to executing processes and their secure access. Topics include memory
  management, concurrent process management, resource management, system call
  implementation, file systems, and memory protection." 4 credits.
- **Prerequisite is CSCI 347.** So C in the UNIX environment is the assumed
  background regardless of who teaches.
- **Silberschatz, Galvin, Gagne, *Operating System Concepts*, 10th ed. (2019)** is
  the text used across WWU 447 sections.
- WWU's systems sequence is `CSCI 247 → 347 → 447`. Nathan has none of it.

## Confirmed about Dr. Tarek Idriss (the actual instructor)

Note the spelling: **Idriss**, two s's.

- Associate Professor, CS. Research in lightweight security, machine learning,
  IoT. Office CF 485. `tarek.idriss@wwu.edu`, (360) 650-4221.
- Has taught CSCI 447 before — reviews exist for Winter 2025 and Spring 2024
  sections. Also teaches CSCI 345.
- **Publishes nothing outside Canvas.** No faculty course pages, no public
  syllabi. This is the central research limitation.

From student reviews of his 447 sections:

- Lecture- and slide-heavy. Exams reportedly track the slides closely; one review
  says the slides were thorough enough to make the textbook unnecessary.
- "Lots of homework" appears on every review. Several single out *one* assignment
  as extremely time-consuming.
- Described as among the hardest courses in the major. Difficulty rated 3–4 of 5.
- Limited office hours (~1.5 hrs/week). Attendance sometimes counted via in-class
  exercises. One review notes extra credit and curved grades.

## NOT confirmed — belongs to Phil Nelson, not the course

Nelson is the *other* 447 instructor and the only one with public materials, so
his version is over-represented in anything findable online. **None of this is
known to apply to Idriss's section:**

- ToyOS / ToyFS as the assignment sequence
- C++ (a deliberately thin subset — no templates, STL, or iostream) as the
  assignment language
- RISC-V 64 under QEMU (`qemu-system-riscv64 -machine virt`) as the target
- No dynamic allocation in the kernel; `kprintf` and a partial `str*`/`mem*` in
  place of libc
- Nelson's published coding standard and AI policy

Most OS courses use C, and department-specific toy kernels don't transfer between
instructors. Idriss may use xv6, a simulator, userspace assignments, or his own
thing. **Resolve via `email-to-idriss.md`.**

## CSCI 347 — settled

Received the Winter 2025 syllabus; it matches Nelson's public Spring 2026 one
nearly line for line, so the topic list is the course's rather than one
instructor's. (The downloaded PDF is misnamed "Computer Organization II"; the
document itself says Computer Systems II.)

- **Text:** Stevens & Rago, *Advanced Programming in the UNIX Environment*, 3rd ed.
- **Topic outline → APUE chapters:** history of UNIX; OS and system calls (Ch 1);
  library calls; man pages; shells, fork, exec, wait; basic UNIX (Ch 1);
  file I/O (Ch 3); files and directories (Ch 4); C standard I/O (Ch 5); system
  information (Ch 6); pipes (Ch 15.2, 15.3); signals (Ch 10); processes
  (Ch 7, 8, 9); concurrency (Ch 11, 12).
- **Assignments in C, not C++.** Six of them, first due about a week into the
  quarter. No IDEs; editor and command line only; must run on the department
  Linux machines.
- Outcomes weight concurrency unusually heavily: "strong understanding of problems
  and techniques in concurrent programming" and "thorough understanding of the
  purpose and use of semaphores, monitors, and rendezvous." That's stronger
  language than any other outcome on the page, and it's why module M6 is sized at
  8–12h rather than being the cut candidate.

## CSCI 247 — settled

Have a day-by-day lecture log from a Fall 2025 student. Full topic map in
`csci247-topic-log.md`. The three open questions resolved as:

- **Neither x86-64 nor RISC-V.** Assembly is taught on a 16-bit toy ISA, and the
  final project is a simulator and assembler for it. So there's no ISA overlap
  with M4b to exploit, and no reason to chase 247's assembly track — it's aimed
  at its own capstone. Take only "can read `objdump -d` of my own C."
- **Memory hierarchy and caching were weighted heavily** — two full weeks (W6–W7)
  on caches, loop optimization, paging, and virtual memory, plus a third (W8) on
  `sbrk`/`malloc` internals. This was the plan's largest hole. Now module **M8**.
- **Linking went as far as ELF files**, one week (W8), alongside the boot process
  the following week. M4a at 4–6h is about right; no change.

Also worth noting: `malloc`/`free`, linked lists, `fork`, threads, and system
calls appear in *both* 247 and 347. Two passes in the prerequisite chain means 509
almost certainly assumes them without reintroduction.

## Environment (verified 2026-08-10)

WSL2, **Ubuntu 22.04.4 LTS**. `git` and `vim` present; **gcc, g++, make, gdb, and
valgrind all absent**, along with the whole RISC-V/QEMU set. `setup/setup-wsl.sh`
closes this.

## Sources

- [CSCI 447 catalog entry](https://catalog.wwu.edu/preview_course_nopop.php?catoid=21&coid=149574)
- [WWU CS core curriculum](https://cs.wwu.edu/csci-core-curriculum)
- [Tarek Idriss faculty page](https://cs.wwu.edu/idrisst)
- [Idriss reviews, RateMyProfessors](https://www.ratemyprofessors.com/professor/2592705)
- [Nelson's CSCI 447 Fall 2025 page](https://facultyweb.cs.wwu.edu/~phil/classes/f25/447/) — Nelson's version only
- [Nelson's CSCI 347 Spring 2026 page](https://facultyweb.cs.wwu.edu/~phil/classes/s26/347/)
- [Nelson's coding standard](https://facultyweb.cs.wwu.edu/~phil/classes/coding.pdf) — not binding for Idriss
- CSCI 347 Winter 2025 syllabus (local PDF, from a classmate)
- CSCI 247 Fall 2025 lecture log (from a classmate) — see `csci247-topic-log.md`
