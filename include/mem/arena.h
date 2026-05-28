#ifndef ARENA_H
#define ARENA_H

#define ARENA_ZERO_UPON_RESET 1

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

typedef uint8_t *BytePtr;
typedef const uint8_t *ReadOnlyBytePtr;

#ifndef ARENA_FATAL_ERROR
#define ARENA_FATAL_ERROR(msg)                                                                     \
    do {                                                                                           \
        (void)fprintf(stderr, "Arena Fatal Error: %s\n", msg);                                     \
        abort();                                                                                   \
    } while (0)
#endif

static inline size_t arena_align_ptr(size_t x)
{
    return ((x) + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
}

typedef struct {
    BytePtr base;
    BytePtr current;
    BytePtr commit_boundary;
    size_t reserve_size;
} Arena;

Arena arena_create(size_t reserve_size);
void *arena_alloc(Arena *a, size_t size);
void arena_reset(Arena *a);
void arena_destroy(Arena *a);

#endif
