#!/bin/sh
# static-vs-shared.sh -- the same code as a .a and as a .so, side by side.
# Nathan Dearing, 2026-08-21.
#
# Two ways to package a library and four things that differ: the size of the
# resulting program, whether the code is inside it, what happens when the
# library changes, and what happens when the library is missing.
#
# Everything is generated into ./libdemo/ and can be deleted.

set -e

CFLAGS="-std=c11 -Wall -Wextra -Werror -g -O0"
d=libdemo
rm -rf "$d"
mkdir "$d"

cat > "$d/greet.c" <<'EOF_C'
#include <stdio.h>

void greet(const char *who)
{
        printf("hello, %s (version 1)\n", who);
}
EOF_C

cat > "$d/greet.h" <<'EOF_C'
#ifndef GREET_H
#define GREET_H
void greet(const char *who);
#endif
EOF_C

cat > "$d/app.c" <<'EOF_C'
#include "greet.h"

int main(void)
{
        greet("world");
        return 0;
}
EOF_C

echo "############################################################"
echo "# static:  ar rcs libgreet.a greet.o"
echo "############################################################"
gcc $CFLAGS -I"$d" -c "$d/greet.c" -o "$d/greet.o"
gcc $CFLAGS -I"$d" -c "$d/app.c"   -o "$d/app.o"
ar rcs "$d/libgreet.a" "$d/greet.o"
gcc "$d/app.o" -L"$d" -lgreet -o "$d/app_static"
echo "an archive is just a bag of .o files with an index:"
ar t "$d/libgreet.a" | sed 's/^/    /'
"$d/app_static"

echo
echo "############################################################"
echo "# shared:  gcc -shared -fPIC -o libgreet.so greet.c"
echo "############################################################"
gcc $CFLAGS -I"$d" -fPIC -c "$d/greet.c" -o "$d/greet_pic.o"
gcc -shared "$d/greet_pic.o" -o "$d/libgreet.so"
gcc "$d/app.o" -L"$d" -lgreet -Wl,-rpath,'$ORIGIN' -o "$d/app_shared"
"$d/app_shared"

echo
echo "############################################################"
echo "# 1. size"
echo "############################################################"
ls -l "$d/app_static" "$d/app_shared" | awk '{printf "  %-28s %8s bytes\n", $9, $5}'
echo
echo "  the static one carries greet's code inside it. The shared one"
echo "  carries a NAME and a promise to find it at startup."

echo
echo "############################################################"
echo "# 2. is the code in there?"
echo "############################################################"
echo "--- nm app_static | grep greet ---"
nm "$d/app_static" | grep -i ' greet' | sed 's/^/    /' || true
echo "--- nm app_shared | grep greet ---"
nm "$d/app_shared" | grep -i ' greet' | sed 's/^/    /' || true
echo
echo "  T in one, U in the other. Same source, same call, different answer"
echo "  to 'where does this function live'."
echo
echo "--- ldd app_static ---"
ldd "$d/app_static" | sed 's/^/    /'
echo "--- ldd app_shared ---"
ldd "$d/app_shared" | sed 's/^/    /'
echo
echo "  (both still need libc.so -- 'static library' here means libgreet was"
echo "   linked in statically, not that the whole program was. A fully static"
echo "   binary needs gcc -static, and then ldd says 'not a dynamic executable'.)"

echo
echo "############################################################"
echo "# 3. change the library WITHOUT rebuilding the program"
echo "############################################################"
sed -i 's/version 1/version 2/' "$d/greet.c"
gcc $CFLAGS -I"$d" -c "$d/greet.c" -o "$d/greet.o"
gcc $CFLAGS -I"$d" -fPIC -c "$d/greet.c" -o "$d/greet_pic.o"
ar rcs "$d/libgreet.a" "$d/greet.o"
gcc -shared "$d/greet_pic.o" -o "$d/libgreet.so"
echo "rebuilt both libraries. Neither app was relinked."
echo
printf '  app_static says:  '; "$d/app_static"
printf '  app_shared says:  '; "$d/app_shared"
echo
echo "  ^ this is the entire argument for shared libraries. A security fix"
echo "    in libc reaches every program on the machine without relinking any"
echo "    of them -- and it is also the entire argument against, because the"
echo "    program you tested is not necessarily the program that runs."

echo
echo "############################################################"
echo "# 4. take the shared library away"
echo "############################################################"
mv "$d/libgreet.so" "$d/libgreet.so.hidden"
printf '  app_static:  '; "$d/app_static"
printf '  app_shared:  '
"$d/app_shared" 2>&1 || true
mv "$d/libgreet.so.hidden" "$d/libgreet.so"
echo
echo "  'error while loading shared libraries' comes from the dynamic loader,"
echo "  before main() runs -- so it is not a link error and not a crash."
echo "  The program was fine; its dependency was not there."
echo
echo "  How the loader decides where to look, in order:"
echo "    DT_RPATH, then LD_LIBRARY_PATH, then DT_RUNPATH, then ld.so.cache,"
echo "    then /lib and /usr/lib."
echo "  This app was linked with -Wl,-rpath,'\$ORIGIN', which means 'next to"
echo "  the executable' -- otherwise you would need LD_LIBRARY_PATH to run it"
echo "  at all. Look at it with:"
echo "    readelf -d $d/app_shared | head -5"
readelf -d "$d/app_shared" | head -5 | sed 's/^/    /'

echo
echo "############################################################"
echo "# 5. why -fPIC"
echo "############################################################"
# Ubuntu's gcc defaults to -fPIE, so an "ordinary" .o here is already
# position-independent and would link into a .so without complaint. Turn
# that default off explicitly to see the error the flag exists to prevent.
gcc $CFLAGS -I"$d" -fno-pic -fno-PIE -c "$d/greet.c" -o "$d/greet_nopic.o"
echo "+ gcc -fno-pic -fno-PIE -c greet.c -o greet_nopic.o"
echo "+ gcc -shared greet_nopic.o -o bad.so"
if gcc -shared "$d/greet_nopic.o" -o "$d/bad.so" 2>"$d/pic.err"; then
        echo "    (it linked anyway on this gcc -- unexpected; see pic.err)"
else
        head -4 "$d/pic.err" | sed 's/^/    /'
fi
echo
cat <<'EOF_PIC'
  A shared library gets mapped at a different address in every process, so
  it cannot contain any absolute address baked in at link time. -fPIC makes
  the compiler emit position-independent code: references go through a table
  that the loader fills in, rather than through a constant.

  A static archive does not need it -- its code is copied into the executable
  and relocated once, at link time, when the final address IS known. That is
  the answer to the last item in the M4a exit test.
EOF_PIC

echo
echo "Everything is in $d/. Delete it whenever."
