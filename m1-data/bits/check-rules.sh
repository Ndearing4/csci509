#!/bin/sh
# check-rules.sh -- flag anything in bits.c that breaks the puzzle rules.
# Nathan Dearing, 2026-08-15.
#
# Strips comments, then looks for banned constructs line by line. It does not
# count operators against each puzzle's budget -- those are in the comments in
# bits.c, count by hand if you are close.

src=${1:-bits.c}

if [ ! -f "$src" ]; then
        echo "no such file: $src" >&2
        exit 2
fi

stripped=`mktemp` || exit 1
trap 'rm -f "$stripped"' EXIT

# -fpreprocessed keeps #include unexpanded while still removing comments,
# so we scan only the code you actually wrote.
gcc -fpreprocessed -dD -E "$src" 2>/dev/null | grep -v '^[[:space:]]*#' > "$stripped"

violations=0

flag() {
        desc=$1
        pat=$2
        sed_pre=${3:-'s/@@nothing@@//'}
        hits=`sed "$sed_pre" "$stripped" | grep -nE "$pat" || true`
        if [ -n "$hits" ]; then
                echo "BANNED: $desc"
                echo "$hits" | sed 's/^/    line /'
                violations=`expr $violations + 1`
        fi
}

echo "=== rule check on $src ==="
echo

flag "loops (for / while / do)"        '\b(for|while|do)\b'
flag "conditionals (if / switch / case / goto)" '\b(if|else|switch|case|goto)\b'
flag "ternary ?:"                      '\?'
flag "logical and/or (&& ||)"          '(&&|\|\|)'
flag "equality/inequality (== !=)"     '(==|!=)'
flag "multiply / divide / modulo"      '[*/%]'
flag "subtraction or unary minus"      '-'
flag "leftover (void) cast from a stub" '\(void\)'

# < and > are only legal as << and >>. Blank the shift operators out first,
# then anything left over is a real comparison.
flag "comparison (< > <= >=)" '[<>]' 's/<<//g; s/>>//g'

# "no other data types" -- everything is plain int.
flag "type other than plain int" '\b(unsigned|signed|long|short|char|float|double)\b'

# "no function calls" -- any identifier immediately followed by "(" that is
# neither a keyword nor a function *definition* header.
calls=''
lineno=0
while IFS= read -r line; do
        lineno=`expr $lineno + 1`
        case "$line" in
        int\ *\(*) continue ;;          # a definition header, not a call
        esac
        for id in `printf '%s\n' "$line" \
                   | grep -oE '[A-Za-z_][A-Za-z0-9_]*[[:space:]]*\(' \
                   | sed 's/[[:space:]]*(//' || true`; do
                case "$id" in
                return|sizeof|if|else|while|for|switch|do) ;;
                *) calls="$calls    line $lineno: $id()
" ;;
                esac
        done
done < "$stripped"

if [ -n "$calls" ]; then
        echo "BANNED: function call"
        printf '%s' "$calls"
        violations=`expr $violations + 1`
fi

# "constants must be 0x00 to 0xFF". Compare by value, not by how it is spelled:
# 0x000000FF is fine, 0x100 is not.
oversized=''
lineno=0
while IFS= read -r line; do
        lineno=`expr $lineno + 1`
        for c in `printf '%s\n' "$line" | grep -oE '0[xX][0-9a-fA-F]+|[0-9]+' || true`; do
                case "$c" in
                0[xX]*)
                        digits=`printf '%s' "$c" | cut -c3- | sed 's/^0*//'`
                        if [ `printf '%s' "$digits" | wc -c` -gt 2 ]; then
                                oversized="$oversized    line $lineno: $c
"
                        fi
                        ;;
                *)
                        if [ "$c" -gt 255 ] 2>/dev/null; then
                                oversized="$oversized    line $lineno: $c
"
                        fi
                        ;;
                esac
        done
done < "$stripped"

if [ -n "$oversized" ]; then
        echo "BANNED: constant larger than 0xFF"
        printf '%s' "$oversized"
        violations=`expr $violations + 1`
fi

echo
if [ "$violations" -eq 0 ]; then
        echo "Clean. No banned constructs found."
        echo "Operator budgets are still yours to check -- see the comments in $src."
else
        echo "$violations rule category/categories violated."
        exit 1
fi
