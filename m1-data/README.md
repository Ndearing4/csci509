# M1 — C's data model · 6–10h

**Skip-ahead probe:** without running it, what does `printf("%zu\n", sizeof(arr))`
print inside a function taking `int arr[10]`, versus in the scope where `arr` was
declared? Why?

Instant answer → skip to M2. Otherwise `bytes/` section 8 demonstrates it.

## The two exercises

```
bits/     twelve datalab-style puzzles, no loops or conditionals
bytes/    print any object's bytes in memory order
```

Do them in parallel, not in sequence. `bits/` is where two's complement, masks,
and shifts stop being abstract; `bytes/` is where you *see* the same facts in
memory. Each makes the other concrete.

### bits/

```sh
cd bits
./build.sh          # compile + run the test harness
./check-rules.sh    # verify you obeyed the operator restrictions
```

`bits.c` has twelve stubs, ordered so each teaches the trick the next assumes.
The rules — no loops, no conditionals, no comparisons, constants capped at
`0xFF` — are the entire exercise. The puzzles are trivial without them.

`test_bits.c` checks every puzzle against a plain-C reference over a table of
nasty edge values (`0`, `INT_MIN`, `0x80000000`, alternating bit patterns) plus
200k deterministic random cases. The seed is fixed, so a failure reproduces
exactly. **Do not read the reference implementations for hints** — they use
loops and conditionals and would only mislead you.

Both checks matter. Passing the tests with an `if` in your solution is not
passing.

### bytes/

```sh
cd bytes
./build.sh
```

Implement one function, `show_bytes`, then the driver walks it through nine
sections: endianness, two's complement, signed-vs-unsigned as pure
interpretation, int vs float representation, why `0.1 + 0.2 != 0.3`, struct
padding, pointers as objects, array decay, and `char` signedness.

The driver runs with the stub in place — you get the commentary and the
`sizeof` output immediately, and the byte lines fill in once the function
works. Each section states what to expect on x86-64 so you can check yourself.

## Budget

The float section is **one hour, no more**. IEEE 754 is heavy in 247 and on its
exams but thin on 509 relevance — enough to not be surprised by it, then move on.

## Exit test

- [x] All twelve puzzles pass `./build.sh`
- [x] `./check-rules.sh` reports clean
- [x] `show_bytes` works on int, float, double, struct, and pointer
- [x] You can explain, without running it, what section 6 prints and why

## Undefined behavior to stay clear of

These bite here specifically, and they bite silently:

- Shifting by less than 0 or more than 31. Puzzle 7 has a trap where the
  obvious mask needs a shift by 32.
- Signed integer overflow. `INT_MAX + 1` is UB, not wraparound — the compiler
  is allowed to assume it never happens and optimize accordingly.
- Reading a struct's padding bytes. Section 6 zeroes them first for exactly
  this reason.
