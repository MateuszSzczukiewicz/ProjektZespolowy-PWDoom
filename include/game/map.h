#ifndef PWDOOM_MAP_H
#define PWDOOM_MAP_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_VERTICES 1024
#define MAX_SECTORS  256
#define MAX_SIDEDEFS 2048
#define MAX_LINEDEFS 2048

typedef struct {
    float x;
    float y;
} Vertex;

typedef struct {
    float floor_height;
    float ceiling_height;
    uint16_t floor_texture;
    uint16_t ceiling_texture;
    uint16_t light_level;
    uint16_t type;
    uint16_t tag;
} Sector;

typedef struct {
    int16_t x_offset;
    int16_t y_offset;
    uint16_t upper_texture;
    uint16_t lower_texture;
    uint16_t middle_texture;
    uint16_t sector;
} Sidedef;

typedef struct {
    uint16_t start_vertex;
    uint16_t end_vertex;
    uint16_t flags;
    uint16_t front_sidedef;
    uint16_t back_sidedef;
} Linedef;

typedef struct {
    Vertex vertices[MAX_VERTICES];
    uint16_t vertex_count;

    Sector sectors[MAX_SECTORS];
    uint16_t sector_count;

    Sidedef sidedefs[MAX_SIDEDEFS];
    uint16_t sidedef_count;

    Linedef linedefs[MAX_LINEDEFS];
    uint16_t linedef_count;
} LevelMap;

float get_floor_height(const LevelMap *map, float x, float z);

void map_init(LevelMap *map);

void map_render(const LevelMap *map);

#endif