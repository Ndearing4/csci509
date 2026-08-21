# The shell, in four stages

Four stages, in this order. Each one ends somewhere you can stop, and each is
debuggable without the next. Skipping ahead means debugging a pipe and a
signal handler at once, and that is how a two-day project becomes a week.

Write `parse.c` completely first. It is pure string handling with no
processes involved, and everything below assumes it is correct.

```sh
./build.sh
./test-shell.sh         # runs the stages you have finished, skips the rest
```

---

## Stage 0 — the parser · ~2h

`parse.c`, both functions, and nothing in `shell.c`.

Finish `parse_dump` too, even though it feels like a detour. You will read its
output a hundred times in stage 3, and the difference between "the pipe does
not work" and "the second command parsed with argc=0" is ten minutes versus an
afternoon.

**Done when:** `./test-shell.sh` passes its parser section, including the five
syntax errors.

---

## Stage 1 — prompt, fork, exec, wait · ~2h

The main loop and `run_simple`, plus the `exit` and `cd` builtins.

This is `procs/spawn.c` inside a `while`. If spawn passed its tests, the only
new things are the loop, `fgets`, and the two builtins.

Two things that are not obvious:

- **The prompt needs `fflush(stdout)`.** Otherwise it sits in the stdio buffer
  and the shell appears to hang before it has done anything. Same mechanism as
  `syscalls/buffering.c` section 1.
- **`cd` cannot be an external program.** Fork a child, chdir, and the child
  exits taking its new directory with it. The shell's own directory never
  moved. That is not a limitation to work around; it is the reason builtins
  exist at all. Write that in a comment where you implement it.

**Done when:** `ls`, `echo hi`, `cd /tmp` then `pwd`, `exit`, and Ctrl-D all
work, and `./test-shell.sh` passes through the builtins section.

---

## Stage 2 — redirection · ~2h

`apply_redirections`, called in the child between `fork` and `execvp`.

That gap is the whole idea. The child is still your program — it can open
files and rearrange its own descriptors — and only then does `exec` replace it
with `ls`, which inherits the descriptors it was handed and has no idea any of
this happened. `ls` has no redirection code in it. Neither does any other
program. That is why `>` is a shell feature.

Get the flags right:

| | |
|---|---|
| `< file` | `O_RDONLY` |
| `> file` | `O_WRONLY \| O_CREAT \| O_TRUNC`, mode `0644` |
| `>> file` | `O_WRONLY \| O_CREAT \| O_APPEND`, mode `0644` |

The mode argument only matters when `O_CREAT` actually creates the file, and
it is masked by your umask — a `0666` there gives you a 0644 file and that is
correct, not a bug.

**Done when:** `ls > out.txt`, `sort < out.txt`, `echo more >> out.txt` all
work, and the shell's own stdout still works afterwards. If your prompt
vanished into out.txt, you did the dup2 in the parent.

---

## Stage 3 — pipes · ~3h, the hard one

`run_pipeline`, for n commands.

Before writing any code, take `a | b | c` and write out the table: two pipes,
four descriptors, four processes counting the shell, and for each process
which of those descriptors it keeps and which it closes. Doing this on paper
takes fifteen minutes and doing it in gdb takes an afternoon.

**It will hang.** Everyone's does. The reason is always the same: a `read` on
a pipe returns EOF only when *no process anywhere* still holds the write end
open, and the shell forked before closing anything, so the shell is still
holding it. The child waits forever for an EOF that the kernel is correct not
to send.

Two rules that between them fix it:

1. Every process closes every pipe descriptor it is not personally using —
   the shell included, and including ends belonging to other commands.
2. Fork **all** the children first, then wait for all of them. Waiting for the
   first before starting the second deadlocks the moment that command writes
   more than a pipe buffer (64 KB on Linux), because nothing is draining the
   other end.

When it hangs anyway: `ls -l /proc/<pid>/fd` on each process shows exactly who
is still holding what, which turns this from guesswork into reading.

The pipeline's exit status is the **last** command's. Reap the others anyway.

**Done when:** `ls -la | grep foo > out.txt` works — the exit test — and so
does a three-stage pipeline, and `head -1` on a big input does not hang.

---

## Stage 4 — Ctrl-C · ~1–2h

Install a `SIGINT` handler with `sigaction`, not `signal`.

`signal`'s behaviour on restarting interrupted system calls differs between
systems and historically between calls; `sigaction` lets you say what you
mean with `SA_RESTART`. Every modern source says use `sigaction`. `man 2
signal` says it about itself, in the notes.

What has to happen:

- Ctrl-C at an empty prompt: the shell prints a newline and a fresh prompt.
  It does not exit.
- Ctrl-C while a child runs: the **child** dies, the shell survives and
  prompts again.
- The shell reports it the way `spawn` did — `128 + SIGINT` is 130.

The second one has a subtlety worth understanding rather than working around.
Ctrl-C sends SIGINT to the whole **foreground process group** — the shell and
the child both. So "the child dies and the shell does not" is not automatic;
the shell has to install a handler that survives, and the child has to be
back on the default action, because it inherited the shell's handler across
`fork` (though not across `exec`, which resets handled signals to default —
work out which of those two facts is doing the work here).

**In the handler, do almost nothing.** Only these are allowed: assign to a
`volatile sig_atomic_t`, and call functions on the async-signal-safe list in
`man 7 signal-safety`. `write` is on it. `printf` is not — if the interrupt
lands while the main flow is inside printf's own bookkeeping, calling printf
again re-enters it on inconsistent state, and the failure is intermittent and
unreproducible, which is the worst kind.

### Manual checklist — `test-shell.sh` cannot do this part

It needs a real terminal, so run these by hand in a real one:

- [ ] `./shell`, then Ctrl-C at the prompt → newline, new prompt, still alive
- [ ] `./shell`, then `sleep 30`, then Ctrl-C → returns to the prompt promptly
- [ ] after that, `ps` shows no `sleep` and no zombie
- [ ] Ctrl-D at the prompt exits cleanly
- [ ] `cat` with no arguments, then Ctrl-C → back at the prompt, not hung

---

## When all four are done

- [ ] `ls -la | grep foo > out.txt`
- [ ] Ctrl-C kills the child, not the shell
- [ ] a hundred commands, then `ps -o stat= -u $(id -u) | grep -c Z` is 0
- [ ] `./test-shell.sh` clean
- [ ] `valgrind ./shell < /dev/null` clean

Then read your `run_pipeline` and answer one question: which line makes
`ls | head -1` terminate instead of hanging when `ls` has more to say? The
answer involves `SIGPIPE`, and it is the thing this whole stage was really
about.
