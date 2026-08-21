# Reading the assembly gcc wrote for you

```sh
./disasm.sh              # add2, at -O0 and -O2
./disasm.sh sum_array    # any function by name
./disasm.sh --all        # everything, into artifacts/
```

Two hours here, reading. Not writing — nothing in M4a asks you to write
assembly, and if you start you have wandered off into 247's track.

The goal is narrow and worth stating: **when gdb stops somewhere with no
source, you should be able to tell roughly where you are.** In an OS course
that is most of the time.

## 1. add2 — the exit test

`./disasm.sh` and read the -O0 version. Twelve instructions for `a + b`.
Annotate every one. Here is the first, to set the format:

```
  0:  endbr64                          a landing-pad marker for control-flow
                                       integrity. Not part of the frame; the
                                       CPU checks indirect jumps land on one.
                                       Ignore it from here on.
  4:  push   rbp                       ← save the CALLER's frame pointer, so
                                       it can be put back on the way out
  5:  mov    rbp,rsp                   ← this frame's anchor: rbp now points
                                       at the base of our frame
  8:  mov    DWORD PTR [rbp-0x14],edi  ← ?
  b:  mov    DWORD PTR [rbp-0x18],esi  ← ?
  e:  mov    edx,DWORD PTR [rbp-0x14]  ← ?
 11:  mov    eax,DWORD PTR [rbp-0x18]  ← ?
 14:  add    eax,edx                   ← ?
 16:  mov    DWORD PTR [rbp-0x4],eax   ← ?
 19:  mov    eax,DWORD PTR [rbp-0x4]   ← ?
 1c:  pop    rbp                       ← ?
 1d:  ret                              ← ?
```

Then answer:

- [ ] Which two slots hold `a` and `b`, and how do you know which is which?
      (The convention is in `disasm.sh`'s output. `edi` is argument one.)
- [ ] Which slot is the local `sum`?
- [ ] Instructions at `16` and `19` store `eax` into `[rbp-0x4]` and
      immediately load it back. **Why would a compiler do that?**
- [ ] The frame uses `[rbp-0x4]` through `[rbp-0x18]` but `rsp` was never
      moved. Where is that memory, and why is it safe to use it here and not
      in a function that calls something else? (Look up "the red zone" — 128
      bytes below `rsp` that a leaf function may use without adjusting it.)
- [ ] `pop rbp` then `ret`. What is on the top of the stack when `ret`
      executes, and who put it there?

## 2. The same function at -O2

Two instructions. `lea eax,[rdi+rsi*1]` — load *effective address*, used here
purely as an add that does not touch the flags.

- [ ] No `push rbp`. No stack slots. No frame at all. What happened to the
      local `sum`?
- [ ] With no saved `rbp` chain, how does a debugger produce a backtrace
      through this function? (`readelf --debug-dump=frames artifacts/frames_O0.o`
      — the answer is the CFI/unwind tables, and this is why release builds
      still give usable stack traces.)
- [ ] **This is why m2-memory and everything else builds with `-g -O0`.**
      Single-step the -O2 version in gdb once, watch the line numbers jump
      backwards, and the reason stops being abstract.

## 3. seven_args — where the seventh one lives

- [ ] Find `a` through `f`. They arrive in `edi esi edx ecx r8d r9d`.
- [ ] Find `g`. It is *not* in a register — it was pushed by the caller, so it
      is at a positive offset from `rbp`, above the saved return address.
- [ ] Sketch the frame: what is at `[rbp+0]`, `[rbp+8]`, `[rbp+16]`? Two of
      those three are the same in every function on this machine.

## 4. sum_array — a loop

- [ ] Find the comparison and the conditional jump. Where is `i`?
- [ ] Find where `values[i]` is computed. There is a scale factor of 4 in the
      addressing. That is pointer arithmetic, in one instruction, and it is
      the same `sizeof` scaling from M1a section 4.
- [ ] `movsx` or `cdqe` will appear somewhere: `int` widened to `long` for the
      `+=`. Integer promotion, visible in the instruction stream.
- [ ] Compare with -O2 and count instructions per iteration.

## 5. swap — pointers in the assembly

- [ ] The arguments are addresses. Count the memory accesses needed to get
      `*x` into a register, versus getting a plain `int` argument.
- [ ] Nothing in the assembly says "pointer". A pointer is just an integer
      the code chose to dereference. That is the whole type system's worth of
      difference, and it is gone by this stage.

## 6. deep — reading a backtrace by hand

```sh
gdb ./artifacts/frames
(gdb) break deep_c
(gdb) run
(gdb) bt
(gdb) info frame
(gdb) x/8gx $rbp
```

- [ ] `bt` shows `deep_c`, `deep_b`, `deep`, `main`. Now find the same chain
      by hand: `$rbp` points at the saved `rbp` of the caller, and the eight
      bytes above it are the return address into that caller.
- [ ] Follow it up two frames with `x/2gx` and check the addresses against
      `info line *0x...`. When you can do that, a corrupted stack becomes
      something you can read rather than something that just says `??`.

## What you should be able to do afterwards

- [ ] Point at the prologue and epilogue of any -O0 function
- [ ] Say where a local lives, and where the first two arguments came from
- [ ] Recognise a call and know what `ret` pops
- [ ] Explain why `-O2` code is harder to debug, in terms of frames rather
      than vaguely
- [ ] Walk one backtrace by hand

That is the whole target. Stop there — writing assembly, learning the full
instruction set, and building an assembler are all 247's track, not this one.
