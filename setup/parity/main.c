/*
 * main.c -- M0 build-parity check.
 *
 * Build and run this in WSL, then build and run it on a department Linux
 * machine, and diff the two outputs. Anything that differs is something that
 * would eventually bite an assignment, found now instead of at 2am in week 3.
 *
 * CSCI 509 prep, module M0. Nathan Dearing, 2026-08-11.
 */

#include <stdio.h>

#include "fingerprint.h"

int main(void)
{
    puts("=== CSCI 509 prep -- M0 build parity check ===");
    putchar('\n');

    print_toolchain();
    print_type_sizes();
    print_byte_order();
    print_memory_layout();

    puts("Built clean and matching on both machines? M0 is done.");

    return 0;
}
