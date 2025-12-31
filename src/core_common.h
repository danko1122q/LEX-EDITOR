#ifndef COMMON_H
#define COMMON_H

// Enable GNU extensions on non-Windows platforms
// This provides access to realpath() and SIGWINCH signal
#ifndef _WIN32
#define _GNU_SOURCE  // realpath, SIGWINCH
#endif

// Standard library includes
#include <stdbool.h>  // bool, true, false
#include <stddef.h>   // size_t, NULL
#include <stdint.h>   // int8_t, uint32_t, etc.
#include <stdio.h>    // FILE, printf, fprintf, etc.
#include <stdlib.h>   // malloc, free, exit, etc.
#include <string.h>   // memcpy, strlen, strcmp, etc.

/**
 * UNUSED - Mark a variable as intentionally unused
 * @x: The variable to mark as unused
 *
 * This macro suppresses compiler warnings about unused variables
 * or parameters. It evaluates to a void expression that references
 * the variable without actually using it.
 *
 * Example:
 *   void function(int used, int unused) {
 *     UNUSED(unused);
 *     return used * 2;
 *   }
 */
#define UNUSED(x) (void) !(x)

/**
 * PANIC - Trigger a panic with file and line information
 * @s: Error message string
 *
 * This macro is a convenience wrapper around the panic() function
 * that automatically passes the current file name and line number.
 * Use this for unrecoverable errors that should terminate the program.
 *
 * Example:
 *   if (critical_error)
 *     PANIC("Critical system failure");
 */
#define PANIC(s) panic(__FILE__, __LINE__, s)

/**
 * panic - Terminate the program with an error message
 * @file: Source file where panic occurred
 * @line: Line number where panic occurred
 * @s: Error message describing the panic reason
 *
 * Prints an error message with location information and terminates
 * the program. Should be used for unrecoverable errors only.
 */
void panic(const char *file, int line, const char *s);

/**
 * malloc_s - Safe malloc with error checking
 * @size: Number of bytes to allocate
 *
 * This macro wraps _malloc_s() and automatically passes file/line info
 * for better error reporting. Panics if allocation fails.
 *
 * Returns: Pointer to allocated memory (never NULL)
 */
#define malloc_s(size) _malloc_s(__FILE__, __LINE__, size)

/**
 * calloc_s - Safe calloc with error checking
 * @n: Number of elements to allocate
 * @size: Size of each element in bytes
 *
 * This macro wraps _calloc_s() and automatically passes file/line info
 * for better error reporting. Allocates zero-initialized memory.
 * Panics if allocation fails.
 *
 * Returns: Pointer to allocated memory (never NULL)
 */
#define calloc_s(n, size) _calloc_s(__FILE__, __LINE__, n, size)

/**
 * realloc_s - Safe realloc with error checking
 * @ptr: Pointer to previously allocated memory (or NULL)
 * @size: New size in bytes
 *
 * This macro wraps _realloc_s() and automatically passes file/line info
 * for better error reporting. Panics if allocation fails.
 *
 * Returns: Pointer to reallocated memory (never NULL)
 */
#define realloc_s(ptr, size) _realloc_s(__FILE__, __LINE__, ptr, size)

/**
 * _malloc_s - Internal safe malloc implementation
 * @file: Source file of the allocation call
 * @line: Line number of the allocation call
 * @size: Number of bytes to allocate
 *
 * Allocates memory and panics with location info if allocation fails.
 * This function should not be called directly; use malloc_s() instead.
 *
 * Returns: Pointer to allocated memory (never NULL)
 */
void *_malloc_s(const char *file, int line, size_t size);

/**
 * _calloc_s - Internal safe calloc implementation
 * @file: Source file of the allocation call
 * @line: Line number of the allocation call
 * @n: Number of elements to allocate
 * @size: Size of each element in bytes
 *
 * Allocates zero-initialized memory and panics with location info if
 * allocation fails. This function should not be called directly;
 * use calloc_s() instead.
 *
 * Returns: Pointer to allocated memory (never NULL)
 */
void *_calloc_s(const char *file, int line, size_t n, size_t size);

/**
 * _realloc_s - Internal safe realloc implementation
 * @file: Source file of the allocation call
 * @line: Line number of the allocation call
 * @ptr: Pointer to previously allocated memory (or NULL)
 * @size: New size in bytes
 *
 * Reallocates memory and panics with location info if allocation fails.
 * This function should not be called directly; use realloc_s() instead.
 *
 * Returns: Pointer to reallocated memory (never NULL)
 */
void *_realloc_s(const char *file, int line, void *ptr, size_t size);

#endif