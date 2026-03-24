#ifndef ARENA_H
# define ARENA_H

# define ARENA_BACKEND			0
# define ARENA_ZERO_UPON_RESET	1

# include <stddef.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# if defined(_WIN32)
#  include <windows.h>
# else
#  include <sys/mman.h>
#  include <unistd.h>
# endif

# ifndef ARENA_FATAL_ERROR
#  define ARENA_FATAL_ERROR(msg) \
	do { fprintf(stderr, "Arena Fatal Error: %s\n", msg); abort(); } while(0)
# endif


# define ARENA_OS_PAGE_SIZE (size_t)4096
# define ARENA_DEFAULT_OVERFLOW ARENA_OS_PAGE_SIZE * 16
# define ARENA_ALIGN_UP_TO_PAGE(x) (((x) + ARENA_OS_PAGE_SIZE - 1) & ~(ARENA_OS_PAGE_SIZE - 1))
# define ARENA_ALIGN_TO_PTR_SIZE(x) (((x) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))

#define DEFAULT_ALIGN sizeof(void*)

# define ARENA_BACKEND_LINUX_MMAP 1
# define ARENA_BACKEND_WIN32_VIRTUALALLOC 2



typedef struct s_arena {
	uint8_t	*base;
	uint8_t	*current;
	uint8_t	*commit_boundary;
	size_t	reserve_size;
}	t_arena;

// -----------------------------------------------------------------------------
// reserve_size is virtual memory and as such it can
// (and in most cases should) be higher than physical RAM size
// set it to a value such that if you were to hit it,
// something has gone terribly wrong
// -----------------------------------------------------------------------------
t_arena arena_create(size_t reserve_size);
void *arena_alloc(t_arena *a, size_t size);
void arena_reset(t_arena *a);
void arena_destroy(t_arena *a);
// -----------------------------------------------------------------------------


#endif