# M0 — build parity check

Three source files, no Makefile. That's deliberate twice over: writing `make` by
hand is **M2's exit test**, and doing it long-hand once first means the Makefile
you write later is describing something you've already seen happen.

## Build

Compile each translation unit separately, then link:

```sh
gcc -std=c11 -Wall -Wextra -Werror -c fingerprint.c -o fingerprint.o
gcc -std=c11 -Wall -Wextra -Werror -c main.c        -o main.o
gcc fingerprint.o main.o -o parity
./parity
```

Note what the flags do and don't apply to. `-Wall -Wextra -Werror` matter on the
**compile** steps, where there's source to diagnose; the final line is a pure
link and takes none of them. `-c` is the flag that says "stop after producing an
object file." Take the `-c` off and gcc runs the whole pipeline through to an
executable — which is why the last line, with no `-c`, produces `parity`.

Object files are not portable. `.o` built in WSL will not link on the department
machine even though both are Linux, unless the architectures and ABI match
exactly. Build from source on each side.

## Run it on both machines

```sh
# in WSL
./parity > /tmp/wsl.txt

# copy the sources up, build there, and capture the same output
scp fingerprint.h fingerprint.c main.c BUILD.md you@dept-host:~/parity/
ssh you@dept-host 'cd ~/parity && gcc -std=c11 -Wall -Wextra -Werror -c fingerprint.c -o fingerprint.o && gcc -std=c11 -Wall -Wextra -Werror -c main.c -o main.o && gcc fingerprint.o main.o -o parity && ./parity' > /tmp/dept.txt

diff /tmp/wsl.txt /tmp/dept.txt
```

## Reading the diff

The **address lines will differ every run on the same machine**, because ASLR
randomizes the layout. That's expected — ignore them and check the *ordering*
instead. To diff only the parts that should be stable:

```sh
diff <(grep -v '0x' /tmp/wsl.txt) <(grep -v '0x' /tmp/dept.txt)
```

What each mismatch would mean:

| Line differs | What it tells you |
|---|---|
| `__GNUC__` / version | Different gcc major version. Watch for warnings that only fire on one side — with `-Werror` that's a build failure on the machine being graded. |
| `__STDC_VERSION__` | One side defaulted to a different standard. Always pass `-std=` explicitly and this stops mattering. |
| any type size | Different architecture or ABI. Genuinely serious: any code assuming `sizeof(long) == 8` or that a pointer fits in an `int` breaks. |
| `plain char is` | x86 vs ARM. Bites the moment you index an array with a `char` holding a high-bit value. |
| byte order | You are on machines of different endianness. Every serialization assumption is now suspect. |

An empty diff on everything but the addresses means the environments agree, and
you can develop in WSL with confidence that what you submit behaves the same way
where it's graded.

## Exit test

- [ ] Builds clean under `-Wall -Wextra -Werror` in WSL
- [ ] Builds clean under the same flags on a department machine
- [ ] Non-address output is identical on both
