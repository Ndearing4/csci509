/*
 * fingerprint.h -- report the properties of the machine and toolchain that a
 * C program can actually observe at runtime.
 *
 * CSCI 509 prep, module M0. Nathan Dearing, 2026-08-11.
 */

#ifndef FINGERPRINT_H
#define FINGERPRINT_H

/* Compiler identity and which C standard it compiled us under. */
void print_toolchain(void);

/* sizeof and related limits for the types whose width C does not fix. */
void print_type_sizes(void);

/* Byte order, determined at runtime rather than assumed. */
void print_byte_order(void);

/* Where the four kinds of storage live, in address order. */
void print_memory_layout(void);

#endif /* FINGERPRINT_H */
