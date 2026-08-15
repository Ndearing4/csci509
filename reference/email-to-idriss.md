# Draft email to Dr. Idriss

**Send this week.** It's the gating item: four factual answers decide roughly 25
hours of scope (modules M4b and M5). Asking in August reads as diligence; the
same email in October reads as panic.

- **To:** tarek.idriss@wwu.edu
- **Subject:** CSCI 509 Fall 2026 — incoming student, question about preparation

Plain text. Keep it short — he has ~1.5 hrs/week of office hours and is
department chair, so respect the inbox.

---

Dear Dr. Idriss,

I'm an incoming MS student registered for CSCI 509 this fall, and I wanted to
introduce myself and ask a preparation question.

My undergraduate degree is in data science rather than computer science, so I
came in without CSCI 247 or 347. I'm comfortable in a UNIX shell and I program
in Python and R, but I have not written C. I have the time between now and
September to close that gap deliberately, and I'd rather aim it at the right
target than guess.

Four things would help me most:

1. What language are the programming assignments written in?
2. Is there a kernel or OS implementation project, and if so what is it built on?
3. Which textbook are you using?
4. What do you most wish students already knew on day one?

I'm not asking for course materials ahead of time — just enough to point my own
preparation in the right direction.

Thank you for your time,

Nathan Dearing

---

## Why these four

| Question | What it decides |
|---|---|
| Language | Whether module M5 (C++ subset, 5–8h) happens at all. Most OS courses use C; the C++ variant at WWU is Phil Nelson's and may not carry over. |
| Kernel project | Whether M4b (bare-metal RISC-V, 10–16h) happens. If it's xv6, that's RISC-V and M4b is close to ideal prep. If assignments are userspace, M4b is a luxury. |
| Textbook | Confirms Silberschatz 10th ed. and lets the M7 reading track the real chapters. |
| Expected background | His own words on the 247/347 gap, which is worth more than any inference from a catalog. |

## When the reply lands

Record the answers in `course-findings.md`, update the module priorities in
`README.md`, and adjust the plan at
`C:\Users\natha\.claude\plans\okay-so-i-need-encapsulated-gadget.md`.

**If no reply by ~Sept 1:** start M4b anyway. xv6-riscv is the most common modern
default, and the debugging skills transfer to any kernel project.
