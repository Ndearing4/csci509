#!/bin/sh
# check-rules.sh -- flag anything in bits.c that breaks the puzzle rules.
# Nathan Dearing, 2026-08-15.
#
# Advisory, not authoritative: it strips comments and looks for banned
# constructs line by line. It does not count operators against each puzzle's
# budget -- the budgets are in the comments in bits.c, count by hand if you
# are close. It also cannot see a function call hiding as a macro.

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
        # flag <pattern-description> <grep-extended-pattern> [pre-filter-sed]
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

echo
if [ "$violations" -eq 0 ]; then
        echo "Clean. No banned constructs found."
        echo "Operator budgets are still yours to check -- see the comments in $src."
else
        echo "$violations rule category/categories violated."
        exit 1
fi
