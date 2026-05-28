#define _DEFAULT_SOURCE
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
    arena.base = mmap(NULL, arena.reserve_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arena.base == MAP_FAILED)
        ARENA_FATAL_ERROR("mmap failed in arena_create");
#endif
    arena.commit_boundary = arena.base;
    arena.current = arena.base;
    return arena;
}

static int arena_commit(Arena *a, size_t need)
{
    size_t page = arena_page_size();
    size_t commit_size = (need + page - 1) & ~(page - 1);
    size_t remaining = (size_t)(a->base + a->reserve_size - a->commit_boundary);
    if (commit_size > remaining)
        commit_size = remaining;
    if (commit_size < page)
        return 0;

#if defined(_WIN32)
    void *p = VirtualAlloc(a->commit_boundary, commit_size, MEM_COMMIT, PAGE_READWRITE);
    if (p == NULL)
        return 0;
#else
    void *p = mmap(a->commit_boundary, commit_size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (p == MAP_FAILED)
        return 0;
#endif
    a->commit_boundary += commit_size;
    return 1;
}

void *arena_alloc(Arena *a, size_t size)
{
    assert(a != NULL);
    if (size == 0)
        return a->current;

    size_t aligned = arena_align_ptr(size);
    if (aligned < size)
        return NULL;

    BytePtr end = a->current + aligned;
    if (end > a->commit_boundary) {
        size_t need = (size_t)(end - a->commit_boundary);
        if (!arena_commit(a, need))
            return NULL;
    }

    void *ptr = a->current;
    a->current = end;
    return ptr;
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
