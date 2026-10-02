#pragma once

/**
 * Compiler attributes used across the decomp.
 *
 * Most of these exist only to reproduce the original code generation: the original build emitted a
 * function out of line (or always inlined it) where this compiler would otherwise decide differently.
 */

/// Never inline the function; the original binary calls it.
#define NOINLINE __attribute__((noinline))

/// Always inline the function; the original binary has it inlined at every call site.
/// This does not imply `inline`: add the keyword where the definition needs it.
#define ALWAYS_INLINE __attribute__((always_inline))

/// Emit the symbol even if nothing references it.
#define USED __attribute__((used))

/// Lay the type out without padding.
#define PACKED __attribute__((packed))

/// Align the declaration to `n` bytes.
#define ALIGNED(n) __attribute__((aligned(n)))

/// Give the symbol hidden visibility.
#define HIDDEN __attribute__((visibility("hidden")))

/// Emit the definition as a weak symbol.
#define WEAK __attribute__((weak))

/// Never turn calls at the end of the function into tail calls.
#define DISABLE_TAIL_CALLS __attribute__((disable_tail_calls))
