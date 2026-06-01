#include "wad/wad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mem/arena.h>

static WadLump* g_lumps = NULL;
static int g_num_lumps = 0;

static int32_t read_i32(const uint8_t* buffer) {
    return ((int32_t)(buffer[0]) | 
           ((int32_t)(buffer[1]) << 8) | 
           ((int32_t)(buffer[2]) << 16) | 
           ((int32_t)(buffer[3]) << 24));
}

static void copy_lump_name(char dest[9], const char src[8]) {
    int i;
    for (i = 0; i < 8 && src[i] != '\0'; i++) {
        dest[i] = (char)toupper((unsigned char)src[i]);
    }
    for (; i < 9; i++) {
        dest[i] = '\0';
    }
}

bool wad_init(const char** paths, int num_files, Arena *arena) {
    g_num_lumps = 0;
    g_lumps = NULL;

    unsigned int total_lumps = 0;
    for (int i = 0; i < num_files; i++) {
        FILE* file = fopen(paths[i], "rb");
        if (!file) {
            fprintf(stderr, "WAD Error: Could not open %s\n", paths[i]);
            continue ; 
        }
        uint8_t header[12];
        if (fread(header, 1, 12, file) != 12 || 
           (memcmp(header, "IWAD", 4) != 0 && memcmp(header, "PWAD", 4) != 0))
        {
            fprintf(stderr, "WAD Error: Invalid signature in %s\n", paths[i]);
            fclose(file);
            continue ;
        }
        total_lumps += (unsigned int)read_i32(header + 4);
        fclose(file);
    }

    if (total_lumps == 0)
        return (false);

    g_lumps = (WadLump*)arena_alloc(arena, total_lumps * sizeof(WadLump));
    if (!g_lumps)
        return (false);

    for (int f = 0; f < num_files; f++) {
        FILE* file = fopen(paths[f], "rb");
        if (!file)
            continue ;

        uint8_t header[12];
        fread(header, 1, 12, file); 
        int32_t num_lumps_in_file = read_i32(header + 4);
        int32_t dir_offset = read_i32(header + 8);
        size_t dir_size = (size_t)num_lumps_in_file * 16;
        uint8_t* dir_buffer = (uint8_t*)malloc(dir_size);
        fseek(file, dir_offset, SEEK_SET);
        if (fread(dir_buffer, 1, dir_size, file) != dir_size) {
            free(dir_buffer);
            fclose(file);
            continue ;
        }
        for (int32_t i = 0; i < num_lumps_in_file; i++) {
            const uint8_t* entry = dir_buffer + (i * 16);
            int32_t lump_offset = read_i32(entry);
            int32_t lump_size = read_i32(entry + 4);
            WadLump* current_lump = &g_lumps[g_num_lumps];
            copy_lump_name(current_lump->name, (const char*)(entry + 8));
            current_lump->size = (size_t)lump_size;
            if (lump_size > 0) {
                uint8_t* lump_data = (uint8_t*)arena_alloc(arena, (size_t)lump_size);
                fseek(file, lump_offset, SEEK_SET);
                fread(lump_data, 1, (size_t)lump_size, file);
                current_lump->data = lump_data;
            } else {
                current_lump->data = NULL;
            }
            g_num_lumps++;
        }
        free(dir_buffer);
        fclose(file);
    }
    return (g_num_lumps > 0);
}

const WadLump* wad_get_lump_by_name(const char* name) {
    char query[9] = {0};
    if (!name || g_num_lumps == 0) return NULL;
    copy_lump_name(query, name);
    for (int i = g_num_lumps - 1; i >= 0; i--)
        if (strncmp(g_lumps[i].name, query, 8) == 0)
            return (&g_lumps[i]);
    return (NULL);
}

const WadLump* wad_get_lump_by_index(int index) {
    if (index < 0 || index >= g_num_lumps) {
        return (NULL);
    }
    return (&g_lumps[index]);
}

int wad_get_lump_count(void) {
    return (g_num_lumps);
}

void wad_forget(void) {
    g_lumps = NULL;
    g_num_lumps = 0;
}

void wad_generate_test(const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("Failed to create test WAD file.\n");
        return ;
    }
    uint8_t color_data[4] = { 43, 43, 86, 255 };
    const char* text_data = "This is a message from a wad file.";
    uint32_t text_len = (uint32_t)strlen(text_data) + 1;
    uint32_t header_size = 12;
    uint32_t offset1 = header_size;
    uint32_t size1 = 4;
    uint32_t offset2 = offset1 + size1;
    uint32_t size2 = text_len;
    fwrite("PWAD", 1, 4, f);
    uint32_t num_lumps = 2;                      
    fwrite(&num_lumps, 4, 1, f);
    uint32_t dir_offset = offset2 + size2;
    fwrite(&dir_offset, 4, 1, f);
    fwrite(color_data, 1, size1, f);
    fwrite(text_data, 1, size2, f);
    char name1[8] = "COLORBG\0";
    fwrite(&offset1, 4, 1, f);
    fwrite(&size1, 4, 1, f);
    fwrite(name1, 1, 8, f);
    char name2[8] = "TEXTMSG\0";
    fwrite(&offset2, 4, 1, f);
    fwrite(&size2, 4, 1, f);
    fwrite(name2, 1, 8, f);
    fclose(f);
    printf("Generated %s successfully.\n", filename);
}

void wad_debug_dump(void) {
    const char* filename = "wad_debug_dump.txt";
    FILE* out = fopen(filename, "w");
    if (!out) {
        printf("ERROR: Could not open %s for writing.\n", filename);
        return ;
    }
    int total_lumps = wad_get_lump_count();
    fprintf(out, "========================================================================\n");
    fprintf(out, " WAD MEMORY DUMP | Total Lumps: %d\n", total_lumps);
    fprintf(out, "========================================================================\n\n");
    for (int i = 0; i < total_lumps; i++) {
        const WadLump* lump = wad_get_lump_by_index(i);
        if (!lump)
            continue ;
        fprintf(out, "------------------------------------------------------------------------\n");
        fprintf(out, " LUMP [%04d] | Name: %-8s | Size: %zu bytes\n", i, lump->name, lump->size);
        fprintf(out, "------------------------------------------------------------------------\n");
        if (lump->size == 0 || !lump->data) {
            fprintf(out, " (Empty / Marker Lump)\n\n");
            continue ;
        }
        fprintf(out, " Offset   | Hex                      | Number (Dec)             | String\n");
        fprintf(out, " ---------+--------------------------+--------------------------+---------\n");
        for (size_t j = 0; j < lump->size; j += 8) {
            fprintf(out, " %08zX | ", j);
            for (size_t k = 0; k < 8; k++) { // hex
                if (j + k < lump->size)
                    fprintf(out, "%02X ", lump->data[j + k]);
                else
                    fprintf(out, "   "); 
            }
            fprintf(out, " | ");
            for (size_t k = 0; k < 8; k++) { // dec
                if (j + k < lump->size)
                    fprintf(out, "%3d ", lump->data[j + k]);
                else
                    fprintf(out, "    "); 
            }
            fprintf(out, " | ");
            for (size_t k = 0; k < 8; k++) { // ascii
                if (j + k < lump->size) {
                    unsigned char c = lump->data[j + k];
                    if (isprint(c))
                        fprintf(out, "%c", c);
                    else
                        fprintf(out, ".");
                }
            }
            fprintf(out, "\n");
        }
        fprintf(out, "\n"); 
    }
    fprintf(out, "\n");
    fprintf(out, "========================================================================\n");
    fprintf(out, " WAD DIRECTORY SUMMARY\n");
    fprintf(out, "========================================================================\n");
    fprintf(out, " Index | Name     | Size (Bytes)\n");
    fprintf(out, " ------+----------+--------------\n");
    for (int i = 0; i < total_lumps; i++) {
        const WadLump* lump = wad_get_lump_by_index(i);
        if (!lump)
            continue ;
        fprintf(out, "  %04d | %-8s | %zu\n", i, lump->name, lump->size);
    }
    fprintf(out, "========================================================================\n");
    fprintf(out, " END OF DUMP\n");
    fprintf(out, "========================================================================\n");
    fclose(out);
    printf("Successfully dumped WAD data to %s\n", filename);
}

void wad_test(void) {
    wad_generate_test("test.wad");
    Arena arena = arena_create((size_t)4096*1024*1024*1024);
    const char *paths[] = {"test.wad", "ACHERON.WAD"};
    wad_init(paths, 2, &arena);
    wad_debug_dump();
    wad_forget();
    arena_destroy(&arena);
}