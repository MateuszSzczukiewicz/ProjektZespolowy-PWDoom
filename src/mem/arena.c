// NOLINTNEXTLINE(bugprone-reserved-identifier,readability-identifier-naming)
#define _GNU_SOURCE
#include "mem/arena.h"

static size_t arena_page_size(void)
{
#if defined(_WIN32)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (size_t)si.dwPageSize;
#else
    long size = sysconf(_SC_PAGE_SIZE);
    if (size <= 0)
        return 4096;
    return (size_t)size;
#endif
}

static size_t arena_align_page(size_t x)
{
    size_t page = arena_page_size();
    return (x + page - 1) & ~(page - 1);
}

Arena arena_create(size_t reserve_size)
{
    Arena arena = {0};
    arena.reserve_size = arena_align_page(reserve_size);
    if (arena.reserve_size == 0)
        arena.reserve_size = arena_page_size();

#if defined(_WIN32)
    arena.base = VirtualAlloc(NULL, arena.reserve_size, MEM_RESERVE, PAGE_NOACCESS);
    if (arena.base == NULL)
        ARENA_FATAL_ERROR("VirtualAlloc failed in arena_create");
#else
    arena.base = mmap(NULL, arena.reserve_size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (arena.base == MAP_FAILED)
        ARENA_FATAL_ERROR("mmap failed in arena_create");
#endif
    arena.commit_boundary = arena.base;
    arena.current = arena.base;
    return (arena);
}

void *arena_alloc(Arena *arena, size_t size)
{
    assert(arena != NULL);

    ReadOnlyBytePtr arena_end = arena->base + arena->reserve_size;
    BytePtr new_current = arena->current + arena_align_ptr(size);

    void *ret = 0;

    if (new_current > arena_end)
        ARENA_FATAL_ERROR("Arena reserve size depleted");
    if (new_current > arena->commit_boundary) {
        size_t needed = (size_t)(new_current - arena->commit_boundary);
        size_t commit_size = arena_align_page(needed + arena_page_size() * 16);
        if (arena->commit_boundary + commit_size > arena_end)
            commit_size = (size_t)(arena_end - arena->commit_boundary);
#if defined(_WIN32)
        const void *result =
            VirtualAlloc(arena->commit_boundary, commit_size, MEM_COMMIT, PAGE_READWRITE);
        if (result == NULL)
            ARENA_FATAL_ERROR("VirtualAlloc failed in arena_alloc");
#else
        const int result = mprotect(arena->commit_boundary, commit_size, PROT_READ | PROT_WRITE);
        if (result != 0)
            ARENA_FATAL_ERROR("mprotect failed in arena_alloc");
#endif
        arena->commit_boundary += commit_size;
    }
    ret = (void *)arena->current;
    arena->current = new_current;
    return (ret);
}

void arena_reset(Arena *arena)
{
    assert(arena != NULL);
    arena->current = arena->base;
#if ARENA_ZERO_UPON_RESET == 1
    size_t committed = (size_t)(arena->commit_boundary - arena->base);
    memset(arena->base, 0, committed);
#endif
}

void arena_destroy(Arena *arena)
{
    assert(arena != NULL);
    if (arena->base == NULL)
        return;
#if defined(_WIN32)
    (void)VirtualFree(arena->base, 0, MEM_RELEASE);
#else
    (void)munmap(arena->base, arena->reserve_size);
#endif
    *arena = (Arena){0};
}
