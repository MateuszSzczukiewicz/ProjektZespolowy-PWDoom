#include "wad/wad.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void write_i32(uint8_t *buffer, int32_t value)
{
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
    buffer[2] = (uint8_t)((value >> 16) & 0xFF);
    buffer[3] = (uint8_t)((value >> 24) & 0xFF);
}

static int generate_test_wad(const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f)
        return 0;

    const uint8_t color_data[4] = {43, 43, 86, 255};
    const char text_data[] = "PWDoom";
    const uint32_t text_len = (uint32_t)strlen(text_data) + 1;
    const uint32_t header_size = 12;
    const uint32_t offset1 = header_size;
    const uint32_t size1 = 4;
    const uint32_t offset2 = offset1 + size1;
    const uint32_t size2 = text_len;
    const uint32_t dir_offset = offset2 + size2;
    uint8_t tmp[4];

    if (fwrite("PWAD", 1, 4, f) != 4)
        goto fail;
    write_i32(tmp, 2);
    if (fwrite(tmp, 4, 1, f) != 1)
        goto fail;
    write_i32(tmp, (int32_t)dir_offset);
    if (fwrite(tmp, 4, 1, f) != 1)
        goto fail;
    if (fwrite(color_data, 1, size1, f) != size1)
        goto fail;
    if (fwrite(text_data, 1, size2, f) != size2)
        goto fail;
    const char name1[8] = "COLORBG";
    write_i32(tmp, (int32_t)offset1);
    if (fwrite(tmp, 4, 1, f) != 1)
        goto fail;
    write_i32(tmp, (int32_t)size1);
    if (fwrite(tmp, 4, 1, f) != 1)
        goto fail;
    if (fwrite(name1, 1, 8, f) != 8)
        goto fail;
    const char name2[8] = "TEXTMSG";
    write_i32(tmp, (int32_t)offset2);
    if (fwrite(tmp, 4, 1, f) != 1)
        goto fail;
    write_i32(tmp, (int32_t)size2);
    if (fwrite(tmp, 4, 1, f) != 1)
        goto fail;
    if (fwrite(name2, 1, 8, f) != 8)
        goto fail;

    (void)fclose(f);
    return 1;

fail:
    (void)fclose(f);
    return 0;
}

int main(void)
{
    printf("WAD parsing tests\n");

    const char *test_wad = "test_wad_tmp.wad";
    if (!generate_test_wad(test_wad)) {
        printf("  FAILED: could not generate test WAD\n");
        return 1;
    }

    Arena arena = arena_create(1 * 1024 * 1024);
    Arena scratch = arena_create(1 * 1024 * 1024);
    assert(arena.base != NULL);
    assert(scratch.base != NULL);

    WadState state = {0};
    const char *paths[] = {test_wad};
    bool ok = wad_init(paths, 1, &arena, &scratch, &state);
    if (!ok) {
        printf("  FAILED: wad_init returned false\n");
        arena_destroy(&arena);
        arena_destroy(&scratch);
        return 1;
    }

    int count = wad_get_lump_count(&state);
    if (count != 2) {
        printf("  FAILED: expected 2 lumps, got %d\n", count);
        arena_destroy(&arena);
        arena_destroy(&scratch);
        return 1;
    }
    printf("  lump count: OK (count=%d)\n", count);

    const WadLump *lump0 = wad_get_lump_by_index(&state, 0);
    if (!lump0 || strcmp(lump0->name, "COLORBG") != 0 || lump0->size != 4) {
        printf("  FAILED: first lump mismatch\n");
        arena_destroy(&arena);
        arena_destroy(&scratch);
        return 1;
    }
    printf("  first lump: OK (name=%s, size=%zu)\n", lump0->name, lump0->size);

    const WadLump *lump1 = wad_get_lump_by_index(&state, 1);
    if (!lump1 || strcmp(lump1->name, "TEXTMSG") != 0 || lump1->size != 7) {
        printf("  FAILED: second lump mismatch\n");
        arena_destroy(&arena);
        arena_destroy(&scratch);
        return 1;
    }
    printf("  second lump: OK (name=%s, size=%zu)\n", lump1->name, lump1->size);

    const WadLump *by_name = wad_get_lump_by_name(&state, "COLORBG");
    if (!by_name || by_name != lump0) {
        printf("  FAILED: lookup by name failed\n");
        arena_destroy(&arena);
        arena_destroy(&scratch);
        return 1;
    }
    printf("  lookup by name: OK\n");

    const WadLump *invalid = wad_get_lump_by_index(&state, 99);
    if (invalid != NULL) {
        printf("  FAILED: out-of-bounds index did not return NULL\n");
        arena_destroy(&arena);
        arena_destroy(&scratch);
        return 1;
    }
    printf("  out-of-bounds: OK\n");

    wad_free(&state);
    arena_destroy(&arena);
    arena_destroy(&scratch);
    (void)remove(test_wad);

    printf("all tests passed\n");
    return 0;
}
