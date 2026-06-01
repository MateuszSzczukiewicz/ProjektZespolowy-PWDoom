#ifndef WAD_H
#define WAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <raylib.h>
#include "mem/arena.h"

typedef struct {
    char name[9];
    const uint8_t* data;
    size_t size;
} WadLump;

bool            wad_init(const char** filepaths, int num_files, Arena* arena);
void            wad_forget(void); // Makes all the data unaccesible but does not reset the arena
const WadLump*  wad_get_lump_by_name(const char* name);
const WadLump*  wad_get_lump_by_index(int index);
int             wad_get_lump_count(void);

void wad_generate_test(const char* filename);
void wad_debug_dump(void);
void wad_test(void);
#endif