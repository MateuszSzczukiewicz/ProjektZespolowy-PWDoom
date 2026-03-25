#include "game/collision.h"

#include "game/doomdef.h"

#include <assert.h>
#include <stddef.h>

#define PLAYER_RADIUS 0.25f
#define MAX_STEP_UP   0.5f

static bool check_position(const LevelMap *map, float x, float z, float current_floor)
{
    assert(map != NULL);

    for (int i = 0; i < map->linedef_count; i++) {
        const Linedef *line = &map->linedefs[i];

        bool blocks = (line->flags & ML_BLOCKING) != 0;

        if (line->back_sidedef != NO_INDEX) {
            uint16_t front_sec = map->sidedefs[line->front_sidedef].sector;
            uint16_t back_sec = map->sidedefs[line->back_sidedef].sector;
            float f_floor = map->sectors[front_sec].floor_height;
            float b_floor = map->sectors[back_sec].floor_height;

            float highest_floor = f_floor > b_floor ? f_floor : b_floor;
            if (highest_floor - current_floor > MAX_STEP_UP) {
                blocks = true;
            }
        } else {
            blocks = true;
        }

        if (!blocks) {
            continue;
        }

        const Vertex *v1 = &map->vertices[line->start_vertex];
        const Vertex *v2 = &map->vertices[line->end_vertex];

        float l2 = (v2->x - v1->x) * (v2->x - v1->x) + (v2->y - v1->y) * (v2->y - v1->y);
        if (l2 == 0.0f) {
            continue;
        }

        float t = ((x - v1->x) * (v2->x - v1->x) + (z - v1->y) * (v2->y - v1->y)) / l2;
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }

        float px = v1->x + t * (v2->x - v1->x);
        float pz = v1->y + t * (v2->y - v1->y);

        float dx = x - px;
        float dz = z - pz;
        float dist_sq = dx * dx + dz * dz;

        if (dist_sq <= PLAYER_RADIUS * PLAYER_RADIUS) {
            return false;
        }
    }

    return true;
}

bool try_move(const LevelMap *map, PlayerState *player, float dx, float dz, float dt)
{
    assert(map != NULL);
    assert(player != NULL);

    float new_x = player->position.x + dx * dt;
    float new_z = player->position.z + dz * dt;

    if (check_position(map, new_x, new_z, player->floor_z)) {
        player->position.x = new_x;
        player->position.z = new_z;
        return true;
    }

    bool moved = false;
    if (check_position(map, new_x, player->position.z, player->floor_z)) {
        player->position.x = new_x;
        player->dz = 0.0f;
        moved = true;
    } else if (check_position(map, player->position.x, new_z, player->floor_z)) {
        player->position.z = new_z;
        player->dx = 0.0f;
        moved = true;
    } else {
        player->dx = 0.0f;
        player->dz = 0.0f;
    }

    return moved;
}
