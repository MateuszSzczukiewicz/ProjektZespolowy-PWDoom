#include "mem/arena.h"

t_arena arena_create(size_t reserve_size)
{
	if (reserve_size == 0) reserve_size = 1;
	t_arena	arena = {0};
	arena.reserve_size = ARENA_ALIGN_UP_TO_PAGE(reserve_size);

	#if defined(_WIN32)
		arena.base = VirtualAlloc(NULL, arena.reserve_size, MEM_RESERVE, PAGE_NOACCESS);
    	if (arena.base == NULL) ARENA_FATAL_ERROR("VirtualAlloc failed in arena_create");
	#else
		arena.base = mmap(NULL, arena.reserve_size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
    	if (arena.base == MAP_FAILED) ARENA_FATAL_ERROR("mmap failed in arena_create");
	#endif
	arena.commit_boundary = arena.base;
	arena.current = arena.base;
	return (arena);
}
void *arena_alloc(t_arena *arena, size_t size)
{
	uint8_t	*new_current = arena->current + ARENA_ALIGN_TO_PTR_SIZE(size);
	uint8_t *arena_end = arena->base + arena->reserve_size;
	size_t	commit_size = 0;
	void	*ret = 0;

	if (new_current > arena_end) ARENA_FATAL_ERROR("Arena reserve size depleted");
	if (new_current > arena->commit_boundary) {
		commit_size = ARENA_ALIGN_UP_TO_PAGE((size_t)(new_current - arena->commit_boundary) + ARENA_DEFAULT_OVERFLOW);
		if (arena->commit_boundary + commit_size > arena_end)
			commit_size = (size_t)(arena_end - arena->commit_boundary);
		#if defined(_WIN32)
    	    void *result = VirtualAlloc(arena->commit_boundary, commit_size, MEM_COMMIT, PAGE_READWRITE);
    	    if (result == NULL) ARENA_FATAL_ERROR("VirtualAlloc failed in arena_alloc");
		#else
    	    int result = mprotect(arena->commit_boundary, commit_size, PROT_READ | PROT_WRITE);
    	    if (result != 0) ARENA_FATAL_ERROR("mprotect failed in arena_alloc");
		#endif
	}
	ret = (void*)arena->current;
	arena->current = new_current;
	arena->commit_boundary += commit_size;
	return (ret);
}
void arena_reset(t_arena *arena)
{
	arena->current = arena->base;
	#if ARENA_ZERO_UPON_RESET == 1
		memset(arena->base, 0, (size_t)(arena->commit_boundary - arena->base)	);
	#endif
}

void arena_destroy(t_arena *arena)
{
	#if defined(_WIN32)
    	(void)VirtualFree(arena->base, 0, MEM_RELEASE);
	#else
    	(void)munmap(arena->base, arena->reserve_size);
	#endif
	*arena = (t_arena){0};
}