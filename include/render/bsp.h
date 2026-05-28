#ifndef PWDOOM_BSP_H
#define PWDOOM_BSP_H

#include "game/map.h"

#include <stdint.h>

#define MAX_BSP_NODES 4096
#define MAX_BSP_SEGS  8192
#define MAX_BSP_LEAVES 2048

typedef struct {
    int32_t front;
    int32_t back;
    float px;
    float py;
    float pdx;
    float pdy;
} BSPNode;

typedef struct {
    Vertex v1;
    Vertex v2;
    uint16_t linedef;
    uint16_t side;
} BSPSeg;

typedef struct {
    uint32_t first;
    uint16_t count;
} BSPLeaf;

typedef struct {
    BSPNode nodes[MAX_BSP_NODES];
    uint16_t node_count;
    BSPSeg segs[MAX_BSP_SEGS];
    uint32_t seg_count;
    uint16_t leaf_segs[MAX_BSP_SEGS];
    uint32_t leaf_seg_count;
    BSPLeaf leaves[MAX_BSP_LEAVES];
    uint16_t leaf_count;
} BSPTree;

void bsp_build(BSPTree *tree, const LevelMap *map);

#endif
