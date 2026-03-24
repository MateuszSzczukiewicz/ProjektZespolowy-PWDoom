#include "mem/arena.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

void run_arena_tests() {
    printf("Starting Arena Tests...\n");

    // We will reserve a small arena for testing: 1 Megabyte
    size_t megabyte = 1024 * 1024;
    t_arena arena = arena_create(megabyte);
    
    // TEST 1: Initialization State
    assert(arena.base != NULL);
    assert(arena.current == arena.base);
    assert(arena.commit_boundary == arena.base);
    assert(arena.reserve_size >= megabyte); 
    printf("[PASS] Initialization\n");

    // TEST 2: Small Allocation & Alignment
    // We request 13 bytes. It should align up to 16 bytes (assuming 8-byte ptrs).
    void *ptr1 = arena_alloc(&arena, 13);
    assert(ptr1 != NULL);
    assert((uintptr_t)ptr1 % sizeof(void*) == 0); // Is it 8-byte aligned?
    
    // Verify pointers moved correctly
    assert(arena.current > arena.base);
    assert(arena.commit_boundary > arena.base); // Boundary must have advanced!
    
    // Prove we can write to it without segfaulting
    strcpy((char*)ptr1, "Hello World!");
    assert(strcmp((char*)ptr1, "Hello World!") == 0);
    printf("[PASS] Small Allocation & Alignment\n");

    // TEST 3: Crossing the Overflow Boundary
    // Force a large allocation to make sure our math doesn't break
    // when committing multiple pages at once.
    size_t large_req = 4096 * 3; 
    void *ptr2 = arena_alloc(&arena, large_req);
    assert(ptr2 != NULL);
    
    // Write to the absolute end of the requested block to prove it's backed by RAM
    uint8_t *large_block = (uint8_t*)ptr2;
    large_block[large_req - 1] = 0xFF; 
    assert(large_block[large_req - 1] == 0xFF);
    printf("[PASS] Large Allocation (Multi-Page Commit)\n");

    // TEST 4: The Reset Mechanism
    size_t old_commit_boundary = (size_t)(arena.commit_boundary - arena.base);
    arena_reset(&arena);
    
    assert(arena.current == arena.base); // Pointer must be rewound
    // The commit boundary should NOT have shrunk! We keep our RAM.
    assert((size_t)(arena.commit_boundary - arena.base) == old_commit_boundary);
    printf("[PASS] Arena Reset\n");

    // TEST 5: Re-allocation (Free RAM usage)
    // Allocating again should NOT trigger OS syscalls because it's already committed
    void *ptr3 = arena_alloc(&arena, 100);
    assert(ptr3 == arena.base); // It should give us the very first byte again
    
    // Prove the commit boundary didn't move because we had plenty of room
    assert((size_t)(arena.commit_boundary - arena.base) == old_commit_boundary);
    printf("[PASS] Re-allocation (No Sycall Triggered)\n");

    // TEST 6: Destruction
    arena_destroy(&arena);
    assert(arena.base == NULL); 
    assert(arena.current == NULL);
    printf("[PASS] Destruction\n");

    printf("\nAll tests passed successfully!\n");
}

int main(void) {
    run_arena_tests();
    return 0;
}