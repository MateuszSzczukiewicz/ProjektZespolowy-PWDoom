#include "game/map.h"

#include "game/doomdef.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

void map_init(LevelMap *map)
{
    assert(map != NULL);
    memset(map, 0, sizeof(*map));

    map->vertex_count = 12;
    map->vertices[0] = (Vertex){0.0f, 0.0f};
    map->vertices[1] = (Vertex){0.0f, 5.0f};
    map->vertices[2] = (Vertex){5.0f, 5.0f};
    map->vertices[3] = (Vertex){5.0f, 3.0f};
    map->vertices[4] = (Vertex){7.0f, 3.0f};
    map->vertices[5] = (Vertex){7.0f, 5.0f};
    map->vertices[6] = (Vertex){12.0f, 5.0f};
    map->vertices[7] = (Vertex){12.0f, 0.0f};
    map->vertices[8] = (Vertex){7.0f, 0.0f};
    map->vertices[9] = (Vertex){7.0f, 2.0f};
    map->vertices[10] = (Vertex){5.0f, 2.0f};
    map->vertices[11] = (Vertex){5.0f, 0.0f};

    map->sector_count = 3;
    map->sectors[0] = (Sector){0.0f, 3.0f, 1, 1, 255, 0, 0};
    map->sectors[1] = (Sector){0.5f, 2.5f, 1, 1, 200, 0, 0};
    map->sectors[2] = (Sector){1.0f, 4.0f, 1, 1, 255, 0, 0};

    map->sidedef_count = 3;
    map->sidedefs[0] = (Sidedef){0, 0, 1, 1, 1, 0};
    map->sidedefs[1] = (Sidedef){0, 0, 1, 1, 1, 1};
    map->sidedefs[2] = (Sidedef){0, 0, 1, 1, 1, 2};

    map->linedef_count = 14;
    map->linedefs[0] = (Linedef){0, 1, ML_BLOCKING, 0, NO_INDEX};
    map->linedefs[1] = (Linedef){1, 2, ML_BLOCKING, 0, NO_INDEX};
    map->linedefs[2] = (Linedef){2, 3, ML_BLOCKING, 0, NO_INDEX};
    map->linedefs[3] = (Linedef){3, 4, ML_BLOCKING, 1, NO_INDEX};
    map->linedefs[4] = (Linedef){4, 5, ML_BLOCKING, 2, NO_INDEX};
    map->linedefs[5] = (Linedef){5, 6, ML_BLOCKING, 2, NO_INDEX};
    map->linedefs[6] = (Linedef){6, 7, ML_BLOCKING, 2, NO_INDEX};
    map->linedefs[7] = (Linedef){7, 8, ML_BLOCKING, 2, NO_INDEX};
    map->linedefs[8] = (Linedef){8, 9, ML_BLOCKING, 2, NO_INDEX};
    map->linedefs[9] = (Linedef){9, 10, ML_BLOCKING, 1, NO_INDEX};
    map->linedefs[10] = (Linedef){10, 11, ML_BLOCKING, 0, NO_INDEX};
    map->linedefs[11] = (Linedef){11, 0, ML_BLOCKING, 0, NO_INDEX};

    map->linedefs[12] = (Linedef){3, 10, ML_TWOSIDED, 0, 1};
    map->linedefs[13] = (Linedef){9, 4, ML_TWOSIDED, 2, 1};
}

static bool point_in_sector(const LevelMap *map, uint16_t sector_idx, float x, float z)
{
    int count = 0;
    for (int i = 0; i < map->linedef_count; i++) {
        const Linedef *line = &map->linedefs[i];

        bool is_front = (line->front_sidedef != NO_INDEX &&
                         map->sidedefs[line->front_sidedef].sector == sector_idx);
        bool is_back = (line->back_sidedef != NO_INDEX &&
                        map->sidedefs[line->back_sidedef].sector == sector_idx);

        if (!is_front && !is_back) {
            continue;
        }

        const Vertex *v1 = &map->vertices[line->start_vertex];
        const Vertex *v2 = &map->vertices[line->end_vertex];

        if ((v1->y > z) != (v2->y > z)) {
            float intersect_x = v1->x + (z - v1->y) * (v2->x - v1->x) / (v2->y - v1->y);
            if (x < intersect_x) {
                count++;
            }
        }
    }
    return (count % 2) != 0;
}

float get_floor_height(const LevelMap *map, float x, float z)
{
    assert(map != NULL);

    for (uint16_t i = 0; i < map->sector_count; i++) {
        if (point_in_sector(map, i, x, z)) {
            return map->sectors[i].floor_height;
        }
    }
    return 0.0f;
}
