#include "wad/wad.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int32_t read_i32(const uint8_t *buffer)
{
    return ((int32_t)(buffer[0]) | ((int32_t)(buffer[1]) << 8) | ((int32_t)(buffer[2]) << 16) |
            ((int32_t)(buffer[3]) << 24));
}

static void copy_lump_name(char dest[9], const char src[8])
{
    int i;
    for (i = 0; i < 8 && src[i] != '\0'; i++) {
        dest[i] = (char)toupper((unsigned char)src[i]);
    }
    for (; i < 9; i++) {
        dest[i] = '\0';
    }
}

bool wad_init(const char **paths, int num_files, Arena *arena, Arena *scratch, WadState *state)
{
    assert(paths != NULL);
    assert(arena != NULL);
    assert(scratch != NULL);
    assert(state != NULL);

    state->count = 0;
    state->lumps = NULL;

    unsigned int total_lumps = 0;
    for (int i = 0; i < num_files; i++) {
        FILE *file = fopen(paths[i], "rb");
        if (!file) {
            (void)fprintf(stderr, "WAD Error: Could not open %s\n", paths[i]);
            continue;
        }
        uint8_t header[12];
        if (fread(header, 1, 12, file) != 12 ||
            (memcmp(header, "IWAD", 4) != 0 && memcmp(header, "PWAD", 4) != 0)) {
            (void)fprintf(stderr, "WAD Error: Invalid signature in %s\n", paths[i]);
            (void)fclose(file);
            continue;
        }
        total_lumps += (unsigned int)read_i32(header + 4);
        (void)fclose(file);
    }

    if (total_lumps == 0)
        return false;

    state->lumps = (WadLump *)arena_alloc(arena, total_lumps * sizeof(WadLump));
    if (!state->lumps)
        return false;

    for (int f = 0; f < num_files; f++) {
        FILE *file = fopen(paths[f], "rb");
        if (!file)
            continue;

        uint8_t header[12];
        if (fread(header, 1, 12, file) != 12) {
            (void)fclose(file);
            continue;
        }
        int32_t num_lumps_in_file = read_i32(header + 4);
        int32_t dir_offset = read_i32(header + 8);
        size_t dir_size = (size_t)num_lumps_in_file * 16;
        uint8_t *dir_buffer = (uint8_t *)arena_alloc(scratch, dir_size);
        if (!dir_buffer) {
            (void)fclose(file);
            continue;
        }
        if (fseek(file, dir_offset, SEEK_SET) != 0) {
            (void)fclose(file);
            continue;
        }
        if (fread(dir_buffer, 1, dir_size, file) != dir_size) {
            (void)fclose(file);
            continue;
        }
        for (int32_t i = 0; i < num_lumps_in_file; i++) {
            const uint8_t *entry = dir_buffer + (ptrdiff_t)(i * 16);
            int32_t lump_offset = read_i32(entry);
            int32_t lump_size = read_i32(entry + 4);
            WadLump *current_lump = &state->lumps[state->count];
            copy_lump_name(current_lump->name, (const char *)(entry + 8));
            current_lump->size = (size_t)lump_size;
            if (lump_size > 0) {
                uint8_t *lump_data = (uint8_t *)arena_alloc(arena, (size_t)lump_size);
                if (lump_data) {
                    if (fseek(file, lump_offset, SEEK_SET) == 0 &&
                        fread(lump_data, 1, (size_t)lump_size, file) == (size_t)lump_size) {
                        current_lump->data = lump_data;
                    } else {
                        current_lump->data = NULL;
                    }
                } else {
                    current_lump->data = NULL;
                }
            } else {
                current_lump->data = NULL;
            }
            state->count++;
        }
        (void)fclose(file);
    }
    return state->count > 0;
}

void wad_free(WadState *state)
{
    if (state) {
        state->lumps = NULL;
        state->count = 0;
    }
}

const WadLump *wad_get_lump_by_name(const WadState *state, const char *name)
{
    assert(state != NULL);
    if (!name || state->count == 0)
        return NULL;
    char query[9] = {0};
    copy_lump_name(query, name);
    for (int i = state->count - 1; i >= 0; i--) {
        if (strncmp(state->lumps[i].name, query, 8) == 0)
            return &state->lumps[i];
    }
    return NULL;
}

const WadLump *wad_get_lump_by_index(const WadState *state, int index)
{
    assert(state != NULL);
    if (index < 0 || index >= state->count) {
        return NULL;
    }
    return &state->lumps[index];
}

int wad_get_lump_count(const WadState *state)
{
    assert(state != NULL);
    return state->count;
}
