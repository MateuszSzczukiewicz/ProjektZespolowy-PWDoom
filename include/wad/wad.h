#ifndef PWDOOM_WAD_H
#define PWDOOM_WAD_H

#include "mem/arena.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    char name[9];
    const uint8_t *data;
    size_t size;
} WadLump;

typedef struct {
    WadLump *lumps;
    int count;
} WadState;

bool wad_init(const char **paths, int num_files, Arena *arena, Arena *scratch, WadState *state);
void wad_free(WadState *state);
const WadLump *wad_get_lump_by_name(const WadState *state, const char *name);
const WadLump *wad_get_lump_by_index(const WadState *state, int index);
int wad_get_lump_count(const WadState *state);

#endif