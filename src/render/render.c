#include "render/render.h"

#include "game/doomdef.h"
#include "raylib.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#define SCREEN_W 1280
#define SCREEN_H 720
#define HALF_W   (SCREEN_W / 2.0f)
#define HALF_H   (SCREEN_H / 2.0f)

#define PROJ_DIST HALF_W

static int16_t upper_clip[SCREEN_W];
static int16_t lower_clip[SCREEN_W];

typedef struct {
    int index;
    float dist_sq;
} RenderSeg;

void render_walls(const LevelMap *map, const PlayerState *player)
{
    assert(map != NULL);
    assert(player != NULL);

    for (int i = 0; i < SCREEN_W; i++) {
        upper_clip[i] = -1;
        lower_clip[i] = SCREEN_H;
    }

    RenderSeg segs[MAX_LINEDEFS];
    for (int i = 0; i < map->linedef_count; i++) {
        segs[i].index = i;
        const Linedef *line = &map->linedefs[i];
        const Vertex *v1 = &map->vertices[line->start_vertex];
        const Vertex *v2 = &map->vertices[line->end_vertex];

        float mid_x = (v1->x + v2->x) * 0.5f;
        float mid_z = (v1->y + v2->y) * 0.5f;

        float dx = mid_x - player->position.x;
        float dz = mid_z - player->position.z;
        segs[i].dist_sq = dx * dx + dz * dz;
    }

    for (int i = 0; i < map->linedef_count - 1; i++) {
        for (int j = i + 1; j < map->linedef_count; j++) {
            if (segs[j].dist_sq < segs[i].dist_sq) {
                RenderSeg temp = segs[i];
                segs[i] = segs[j];
                segs[j] = temp;
            }
        }
    }

    float cos_a = cosf(player->angle);
    float sin_a = sinf(player->angle);

    for (int i = 0; i < map->linedef_count; i++) {
        const Linedef *line = &map->linedefs[segs[i].index];
        const Vertex *v1 = &map->vertices[line->start_vertex];
        const Vertex *v2 = &map->vertices[line->end_vertex];

        float dx1 = v1->x - player->position.x;
        float dz1 = v1->y - player->position.z;
        float dx2 = v2->x - player->position.x;
        float dz2 = v2->y - player->position.z;

        float tx1 = dx1 * cos_a - dz1 * sin_a;
        float tz1 = dx1 * sin_a + dz1 * cos_a;

        float tx2 = dx2 * cos_a - dz2 * sin_a;
        float tz2 = dx2 * sin_a + dz2 * cos_a;

        if (tz1 < 0.1f && tz2 < 0.1f) {
            continue;
        }

        if (tz1 < 0.1f) {
            float t = (0.1f - tz1) / (tz2 - tz1);
            tx1 = tx1 + t * (tx2 - tx1);
            tz1 = 0.1f;
        } else if (tz2 < 0.1f) {
            float t = (0.1f - tz2) / (tz1 - tz2);
            tx2 = tx2 + t * (tx1 - tx2);
            tz2 = 0.1f;
        }

        float xscale1 = PROJ_DIST / tz1;
        float xscale2 = PROJ_DIST / tz2;

        int sx1 = (int)(HALF_W + tx1 * xscale1);
        int sx2 = (int)(HALF_W + tx2 * xscale2);

        if (sx1 >= sx2) {
            continue;
        }

        int draw_x1 = sx1 < 0 ? 0 : sx1;
        int draw_x2 = sx2 > SCREEN_W - 1 ? SCREEN_W - 1 : sx2;

        if (draw_x1 > draw_x2) {
            continue;
        }

        float ceil_h = 3.0f, floor_h = 0.0f;
        float back_ceil_h = 0.0f, back_floor_h = 0.0f;
        bool is_portal = line->back_sidedef != NO_INDEX;

        if (line->front_sidedef != NO_INDEX) {
            uint16_t sec = map->sidedefs[line->front_sidedef].sector;
            ceil_h = map->sectors[sec].ceiling_height;
            floor_h = map->sectors[sec].floor_height;
        }
        if (is_portal) {
            uint16_t back_sec = map->sidedefs[line->back_sidedef].sector;
            back_ceil_h = map->sectors[back_sec].ceiling_height;
            back_floor_h = map->sectors[back_sec].floor_height;
        }

        float y_ceil1 = HALF_H - (ceil_h - player->view_z) * xscale1;
        float y_floor1 = HALF_H - (floor_h - player->view_z) * xscale1;
        float y_ceil2 = HALF_H - (ceil_h - player->view_z) * xscale2;
        float y_floor2 = HALF_H - (floor_h - player->view_z) * xscale2;

        float y_bceil1 = 0.0f, y_bfloor1 = 0.0f, y_bceil2 = 0.0f, y_bfloor2 = 0.0f;
        if (is_portal) {
            y_bceil1 = HALF_H - (back_ceil_h - player->view_z) * xscale1;
            y_bfloor1 = HALF_H - (back_floor_h - player->view_z) * xscale1;
            y_bceil2 = HALF_H - (back_ceil_h - player->view_z) * xscale2;
            y_bfloor2 = HALF_H - (back_floor_h - player->view_z) * xscale2;
        }

        float dx_screen = (float)(sx2 - sx1);
        float step_ceil = (y_ceil2 - y_ceil1) / dx_screen;
        float step_floor = (y_floor2 - y_floor1) / dx_screen;

        float step_bceil = 0.0f, step_bfloor = 0.0f;
        if (is_portal) {
            step_bceil = (y_bceil2 - y_bceil1) / dx_screen;
            step_bfloor = (y_bfloor2 - y_bfloor1) / dx_screen;
        }

        float current_y_ceil = y_ceil1 + (float)(draw_x1 - sx1) * step_ceil;
        float current_y_floor = y_floor1 + (float)(draw_x1 - sx1) * step_floor;
        float current_y_bceil = 0.0f, current_y_bfloor = 0.0f;

        if (is_portal) {
            current_y_bceil = y_bceil1 + (float)(draw_x1 - sx1) * step_bceil;
            current_y_bfloor = y_bfloor1 + (float)(draw_x1 - sx1) * step_bfloor;
        }

        for (int x = draw_x1; x <= draw_x2; x++) {
            int yc = (int)current_y_ceil;
            int yf = (int)current_y_floor;

            int draw_yc = yc;
            int draw_yf = yf;

            if (draw_yc < upper_clip[x] + 1)
                draw_yc = upper_clip[x] + 1;
            if (draw_yf > lower_clip[x] - 1)
                draw_yf = lower_clip[x] - 1;

            if (upper_clip[x] + 1 < yc) {
                int limit = yc > lower_clip[x] ? lower_clip[x] : yc;
                int start = upper_clip[x] + 1;
                if (start < limit) {
                    DrawLine(x, start, x, limit, LIGHTGRAY);
                }
            }

            if (lower_clip[x] - 1 > yf) {
                int start = yf < upper_clip[x] ? upper_clip[x] : yf;
                int limit = lower_clip[x] - 1;
                if (start < limit) {
                    DrawLine(x, start, x, limit, BROWN);
                }
            }

            if (is_portal) {
                int ybc = (int)current_y_bceil;
                int ybf = (int)current_y_bfloor;

                if (ybc > draw_yc) {
                    int h = ybc > draw_yf ? draw_yf : ybc;
                    if (h >= draw_yc) {
                        DrawLine(x, draw_yc, x, h, GRAY);
                        if (h > upper_clip[x])
                            upper_clip[x] = (int16_t)h;
                    }
                }

                if (ybf < draw_yf) {
                    int l = ybf < draw_yc ? draw_yc : ybf;
                    if (l <= draw_yf) {
                        DrawLine(x, l, x, draw_yf, GRAY);
                        if (l < lower_clip[x])
                            lower_clip[x] = (int16_t)l;
                    }
                }
            } else {
                if (draw_yc <= draw_yf) {
                    DrawLine(x, draw_yc, x, draw_yf, DARKGRAY);
                    upper_clip[x] = SCREEN_H;
                }
            }

            current_y_ceil += step_ceil;
            current_y_floor += step_floor;
            if (is_portal) {
                current_y_bceil += step_bceil;
                current_y_bfloor += step_bfloor;
            }
        }
    }
}
