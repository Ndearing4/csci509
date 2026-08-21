#!/bin/sh
# reveal.sh -- show where each declaration in sections.c actually landed.
# Nathan Dearing, 2026-08-21.
#
# Fill in SECTIONS.md first. This script is the answer key and it is much
# less useful read in the other order.

set -e

CFLAGS="-std=c11 -Wall -Wextra -Werror -g -O0"

echo "+ gcc $CFLAGS -c sections.c -o sections.o"
gcc $CFLAGS -c sections.c -o sections.o

echo
echo "=== nm: symbol, letter, and what the letter means ==="
echo
nm sections.o | while read -r addr letter name; do
        [ -n "$name" ] || { name=$letter; letter=$addr; }
        case "$letter" in
        T) meaning="global function        (.text)" ;;
        t) meaning="static function        (.text, local)" ;;
        D) meaning="global, nonzero init   (.data)" ;;
        d) meaning="static, nonzero init   (.data, local)" ;;
        B) meaning="global, zero init      (.bss)" ;;
        b) meaning="static, zero init      (.bss, local)" ;;
        R) meaning="global const           (.rodata)" ;;
        r) meaning="static const           (.rodata, local)" ;;
        U) meaning="undefined              (someone else has it)" ;;
        C) meaning="common                 (pre -fno-common; rare now)" ;;
        *) meaning="section code '$letter'" ;;
        esac
        printf '  %-3s %-24s %s\n' "$letter" "$name" "$meaning"
done

echo
echo "=== readelf: which section each symbol is really in ==="
echo
readelf -sW sections.o | awk '
        NR<=3 { next }
        $8 != "" && $4 != "FILE" && $4 != "SECTION" && $8 !~ /^\$/ {
                printf "  %-24s size %-6s %s\n", $8, $3, $7
        }' | sort

echo
echo "  (the last column is a section INDEX. Map it with the table below,"
echo "   or read the Ndx column of readelf -s directly.)"
echo
readelf -SW sections.o \
        | sed 's/\[ *\([0-9]*\)\]/[\1]/' \
        | awk '/^ *\[[0-9]+\]/ { printf "  %-5s %-22s %s\n", $1, $2, $3 }'

echo
echo "  ^ note .bss is type NOBITS while every other section is PROGBITS."
echo "    NOBITS means exactly what it says: the section header records a"
echo "    size and an address, and the file contains none of the bytes."
echo
echo "=== size: what it costs on disk ==="
echo
size sections.o
echo
echo "  text = code and .rodata.  data = initialised writable.  bss = zeros."
echo "  bss is a NUMBER in the file, not bytes. golf[1024] is in there and"
echo "  costs nothing; hotel[1024] is in data and costs 1024."

echo
echo "=== the local that has no symbol at all ==="
echo
if nm sections.o | grep -q oscar; then
        echo "  'oscar' IS in the symbol table -- unexpected, look at why"
else
        echo "  'oscar' is nowhere in nm's output. An ordinary local has no"
        echo "  symbol, because it has no address until the function runs and"
        echo "  a different address every time it is called. The linker has"
        echo "  nothing to link. Only -g debug info knows the name exists:"
        echo
        echo "      objdump --dwarf=info sections.o | grep -A2 oscar"
fi
echo
nm sections.o | grep -i novemb | sed 's/^/  /' || true
echo "  ^ and there is the static local, with a mangled name so that two"
echo "    files can each have one without colliding."

echo
echo "Now go back to SECTIONS.md and check the ones you got wrong."
