#!/usr/bin/env bash
#
# setup-wsl.sh -- CSCI 509 prep toolchain for Ubuntu 22.04 (WSL2).
# Nathan Dearing, started 2026-08-10.
#
# Idempotent: installs only what is missing, safe to re-run.
#
#   ./setup-wsl.sh                 core C toolchain (modules M0-M4a, M6)
#   ./setup-wsl.sh --with-riscv    also RISC-V cross-compiler + QEMU (module M4b)
#
# M4b is conditional on what Dr. Idriss confirms about the assignments, so the
# RISC-V half is opt-in rather than default.

set -euo pipefail

# Core toolchain. manpages-dev matters more than it looks: it provides the
# section 2 and 3 man pages, and the syscall-vs-library distinction
# (man 2 read vs man 3 printf) is course material, not trivia.
CORE_PKGS=(
  build-essential   # gcc, g++, make, libc headers
  gdb
  valgrind
  manpages-dev
  git
  vim
)

# M4b only.
RISCV_PKGS=(
  qemu-system-misc        # provides qemu-system-riscv64
  gdb-multiarch           # gdb that speaks RISC-V, for QEMU's gdbstub
  gcc-riscv64-unknown-elf # freestanding (bare-metal) cross compiler
)

want_riscv=0
case "${1:-}" in
  --with-riscv) want_riscv=1 ;;
  "")           ;;
  -h|--help)    sed -n '3,12p' "$0"; exit 0 ;;
  *)            echo "unknown option: $1" >&2; exit 2 ;;
esac

pkgs=("${CORE_PKGS[@]}")
if (( want_riscv )); then
  pkgs+=("${RISCV_PKGS[@]}")
fi

# Only touch apt if something is actually absent.
missing=()
for p in "${pkgs[@]}"; do
  if ! dpkg -s "$p" >/dev/null 2>&1; then
    missing+=("$p")
  fi
done

if (( ${#missing[@]} > 0 )); then
  echo "Installing: ${missing[*]}"
  sudo apt-get update
  sudo apt-get install -y "${missing[@]}"
else
  echo "All requested packages already present."
fi

echo
echo "--- verification ---"

check() {
  local tool=$1
  if command -v "$tool" >/dev/null 2>&1; then
    printf 'OK       %-24s %s\n' "$tool" "$("$tool" --version 2>&1 | head -n1)"
  else
    printf 'MISSING  %s\n' "$tool"
    return 1
  fi
}

rc=0
for t in gcc g++ make gdb valgrind git vim; do
  check "$t" || rc=1
done

if (( want_riscv )); then
  for t in qemu-system-riscv64 gdb-multiarch riscv64-unknown-elf-gcc; do
    check "$t" || rc=1
  done
fi

# man 2 / man 3 are only useful if the dev pages actually landed.
if man 2 read >/dev/null 2>&1; then
  echo "OK       man section 2 available"
else
  echo "MISSING  man section 2 (syscall pages)"
  rc=1
fi

echo
if (( rc == 0 )); then
  echo "Toolchain ready."
else
  echo "Some tools are missing -- see above." >&2
fi
exit $rc
