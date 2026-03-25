#include "game/collision.h"

#include "game/doomdef.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>

#define PLAYER_RADIUS 0.25f
#define MAX_STEP_UP   0.5f

static const Linedef *get_blocking_line(const LevelMap *map, float x, float z, float current_floor)
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

        float lx = v2->x - v1->x;
        float lz = v2->y - v1->y;
        float l2 = lx * lx + lz * lz;

        if (l2 == 0.0f) {
            continue;
        }

        float t = ((x - v1->x) * lx + (z - v1->y) * lz) / l2;
        if (t < 0.0f) {
            t = 0.0f;
        } else if (t > 1.0f) {
            t = 1.0f;
        }

        float px = v1->x + t * lx;
        float pz = v1->y + t * lz;

        float dist_sq = (x - px) * (x - px) + (z - pz) * (z - pz);

        if (dist_sq <= PLAYER_RADIUS * PLAYER_RADIUS) {
            return line;
        }
    }

    return NULL;
}

bool try_move(const LevelMap *map, PlayerState *player, float dx, float dz, float dt)
{
    assert(map != NULL);
    assert(player != NULL);

    float move_x = dx * dt;
    float move_z = dz * dt;
    float new_x = player->position.x + move_x;
    float new_z = player->position.z + move_z;

    const Linedef *blocker = get_blocking_line(map, new_x, new_z, player->floor_z);

    if (!blocker) {
        player->position.x = new_x;
        player->position.z = new_z;
        return true;
    }

    const Vertex *v1 = &map->vertices[blocker->start_vertex];
    const Vertex *v2 = &map->vertices[blocker->end_vertex];
    float lx = v2->x - v1->x;
    float lz = v2->y - v1->y;
    float len = sqrtf(lx * lx + lz * lz);

    if (len > 0.0f) {
        float tx = lx / len;
        float tz = lz / len;

        float dot_move = move_x * tx + move_z * tz;
        float p_move_x = tx * dot_move;
        float p_move_z = tz * dot_move;

        if (!get_blocking_line(map, player->position.x + p_move_x, player->position.z + p_move_z,
                               player->floor_z)) {
            player->position.x += p_move_x;
            player->position.z += p_move_z;
        }

        float dot_vel = player->dx * tx + player->dz * tz;
        player->dx = tx * dot_vel;
        player->dz = tz * dot_vel;
        return true;
    }

    player->dx = 0.0f;
    player->dz = 0.0f;
    return false;
}
