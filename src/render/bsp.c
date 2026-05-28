#include "render/bsp.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>

#define LEAF_SIZE 4

static float point_side(float px, float py, float lx, float ly, float dx, float dy)
{
    return (px - lx) * dy - (py - ly) * dx;
}

static int point_class(float px, float py, float lx, float ly, float dx, float dy)
{
    float s = point_side(px, py, lx, ly, dx, dy);
    if (s > 0.001f)
        return 1;
    if (s < -0.001f)
        return -1;
    return 0;
}

static void intersect(float ax, float ay, float adx, float ady,
                      float bx, float by, float bdx, float bdy,
                      float *ox, float *oy)
{
    float denom = adx * bdy - ady * bdx;
    if (fabsf(denom) < 0.0001f) {
        *ox = (ax + bx) * 0.5f;
        *oy = (ay + by) * 0.5f;
        return;
    }
    float t = ((bx - ax) * bdy - (by - ay) * bdx) / denom;
    *ox = ax + t * adx;
    *oy = ay + t * ady;
}

static int32_t build_node(BSPTree *tree, const uint16_t *indices, uint16_t count, Arena *scratch)
{
    if (count <= LEAF_SIZE) {
        uint32_t first = tree->leaf_seg_count;
        for (uint16_t i = 0; i < count; i++)
            tree->leaf_segs[tree->leaf_seg_count++] = indices[i];
        uint16_t leaf = tree->leaf_count;
        tree->leaves[leaf].first = first;
        tree->leaves[leaf].count = count;
        tree->leaf_count++;
        return (int32_t)(-leaf - 1);
    }

    int best_idx = -1;
    int best_score = -1;
    for (uint16_t i = 0; i < count; i++) {
        const BSPSeg *seg = &tree->segs[indices[i]];
        float dx = seg->v2.x - seg->v1.x;
        float dy = seg->v2.y - seg->v1.y;
        float len = sqrtf(dx * dx + dy * dy);
        if (len < 0.001f)
            continue;
        dx /= len;
        dy /= len;
        int front = 0, back = 0, span = 0;
        for (uint16_t j = 0; j < count; j++) {
            const BSPSeg *sj = &tree->segs[indices[j]];
            int s1 = point_class(sj->v1.x, sj->v1.y, seg->v1.x, seg->v1.y, dx, dy);
            int s2 = point_class(sj->v2.x, sj->v2.y, seg->v1.x, seg->v1.y, dx, dy);
            if (s1 == 0 && s2 == 0) {
            } else if (s1 == s2) {
                if (s1 == 1)
                    front++;
                else if (s1 == -1)
                    back++;
            } else {
                span++;
            }
        }
        if (front == 0 || back == 0)
            continue;
        int imbalance = front > back ? front - back : back - front;
        int score = imbalance + span * 4;
        if (best_score < 0 || score < best_score) {
            best_score = score;
            best_idx = i;
        }
    }
    if (best_idx < 0) {
        uint32_t first = tree->leaf_seg_count;
        for (uint16_t i = 0; i < count; i++)
            tree->leaf_segs[tree->leaf_seg_count++] = indices[i];
        uint16_t leaf = tree->leaf_count;
        tree->leaves[leaf].first = first;
        tree->leaves[leaf].count = count;
        tree->leaf_count++;
        return (int32_t)(-leaf - 1);
    }

    const BSPSeg *part = &tree->segs[indices[best_idx]];
    float pdx = part->v2.x - part->v1.x;
    float pdy = part->v2.y - part->v1.y;
    float plen = sqrtf(pdx * pdx + pdy * pdy);
    pdx /= plen;
    pdy /= plen;

    uint16_t front_stack[MAX_BSP_SEGS];
    uint16_t back_stack[MAX_BSP_SEGS];
    uint16_t *front_idx = front_stack;
    uint16_t *back_idx = back_stack;
    uint16_t fc = 0, bc = 0;

    if (scratch != NULL) {
        front_idx = arena_alloc(scratch, count * sizeof(uint16_t));
        back_idx = arena_alloc(scratch, count * sizeof(uint16_t));
    }

    for (uint16_t i = 0; i < count; i++) {
        const BSPSeg *seg = &tree->segs[indices[i]];
        int s1 = point_class(seg->v1.x, seg->v1.y, part->v1.x, part->v1.y, pdx, pdy);
        int s2 = point_class(seg->v2.x, seg->v2.y, part->v1.x, part->v1.y, pdx, pdy);

        if (s1 >= 0 && s2 >= 0) {
            front_idx[fc++] = indices[i];
        } else if (s1 <= 0 && s2 <= 0) {
            back_idx[bc++] = indices[i];
        } else {
            float ix, iy;
            float edx = seg->v2.x - seg->v1.x;
            float edy = seg->v2.y - seg->v1.y;
            intersect(seg->v1.x, seg->v1.y, edx, edy,
                      part->v1.x, part->v1.y, pdx, pdy, &ix, &iy);

            uint16_t orig = indices[i];
            Vertex orig_v1 = seg->v1;
            Vertex orig_v2 = seg->v2;
            uint16_t new_idx = (uint16_t)tree->seg_count++;

            tree->segs[new_idx].linedef = tree->segs[orig].linedef;

            if (s1 > 0) {
                tree->segs[orig].v1 = orig_v1;
                tree->segs[orig].v2 = (Vertex){ix, iy};
                tree->segs[new_idx].v1 = (Vertex){ix, iy};
                tree->segs[new_idx].v2 = orig_v2;
            } else {
                tree->segs[orig].v1 = (Vertex){ix, iy};
                tree->segs[orig].v2 = orig_v2;
                tree->segs[new_idx].v1 = orig_v1;
                tree->segs[new_idx].v2 = (Vertex){ix, iy};
            }

            front_idx[fc++] = orig;
            back_idx[bc++] = new_idx;
        }
    }

    int32_t front_child = build_node(tree, front_idx, fc, scratch);
    int32_t back_child = build_node(tree, back_idx, bc, scratch);

    uint16_t node = tree->node_count;
    tree->nodes[node].front = front_child;
    tree->nodes[node].back = back_child;
    tree->nodes[node].px = part->v1.x;
    tree->nodes[node].py = part->v1.y;
    tree->nodes[node].pdx = pdx;
    tree->nodes[node].pdy = pdy;
    tree->node_count++;

    return (int32_t)node;
}

void bsp_build(BSPTree *tree, const LevelMap *map, Arena *scratch)
{
    assert(tree != NULL);
    assert(map != NULL);

    tree->node_count = 0;
    tree->seg_count = 0;
    tree->leaf_count = 0;
    tree->leaf_seg_count = 0;

    if (map->linedef_count == 0)
        return;

    for (uint16_t i = 0; i < map->linedef_count; i++) {
        const Linedef *line = &map->linedefs[i];
        tree->segs[i].v1 = map->vertices[line->start_vertex];
        tree->segs[i].v2 = map->vertices[line->end_vertex];
        tree->segs[i].linedef = i;
    }
    tree->seg_count = map->linedef_count;

    uint16_t init_idx[MAX_LINEDEFS];
    for (uint16_t i = 0; i < map->linedef_count; i++)
        init_idx[i] = i;

    build_node(tree, init_idx, map->linedef_count, scratch);
}
