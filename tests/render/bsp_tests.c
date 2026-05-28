#include "render/bsp.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void test_bsp_build_counts(void)
{
    LevelMap map;
    map_init(&map);

    BSPTree tree;
    bsp_build(&tree, &map);

    assert(tree.node_count >= 1);
    assert(tree.leaf_count >= 1);
    assert(tree.seg_count >= map.linedef_count);
    assert(tree.leaf_seg_count <= tree.seg_count);

    uint32_t total_leaf = 0;
    for (uint16_t i = 0; i < tree.leaf_count; i++)
        total_leaf += tree.leaves[i].count;
    assert(total_leaf == tree.leaf_seg_count);

    printf("  build counts: OK (nodes=%u, leaves=%u, segs=%u, leaf_segs=%u)\n",
           tree.node_count, tree.leaf_count, tree.seg_count, tree.leaf_seg_count);
}

static void test_bsp_no_stale_segs(void)
{
    LevelMap map;
    map_init(&map);

    BSPTree tree;
    bsp_build(&tree, &map);

    uint8_t referenced[MAX_BSP_SEGS];
    memset(referenced, 0, sizeof(referenced));

    for (uint16_t i = 0; i < tree.leaf_count; i++) {
        uint32_t end = tree.leaves[i].first + tree.leaves[i].count;
        for (uint32_t j = tree.leaves[i].first; j < end; j++) {
            uint16_t seg_idx = tree.leaf_segs[j];
            assert(seg_idx < tree.seg_count);
            referenced[seg_idx] = 1;
        }
    }

    for (uint32_t i = 0; i < tree.seg_count; i++)
        assert(referenced[i] == 1);

    printf("  no stale segs: OK\n");
}

static void test_bsp_node_children(void)
{
    LevelMap map;
    map_init(&map);

    BSPTree tree;
    bsp_build(&tree, &map);

    for (uint16_t i = 0; i < tree.node_count; i++) {
        int32_t f = tree.nodes[i].front;
        int32_t b = tree.nodes[i].back;
        uint16_t leaf_idx;

        if (f >= 0) {
            assert((uint16_t)f < tree.node_count);
        } else {
            leaf_idx = (uint16_t)(-f - 1);
            assert(leaf_idx < tree.leaf_count);
        }

        if (b >= 0) {
            assert((uint16_t)b < tree.node_count);
        } else {
            leaf_idx = (uint16_t)(-b - 1);
            assert(leaf_idx < tree.leaf_count);
        }
    }

    printf("  node children valid: OK\n");
}

static void test_bsp_root_is_valid(void)
{
    LevelMap map;
    map_init(&map);

    BSPTree tree;
    bsp_build(&tree, &map);

    if (tree.node_count > 0) {
        assert(tree.nodes[0].front != 0);
        assert(tree.nodes[0].back != 0);
    } else {
        assert(tree.leaf_count == 1);
    }

    printf("  root node valid: OK\n");
}

int main(void)
{
    printf("BSP tree tests\n");
    test_bsp_build_counts();
    test_bsp_no_stale_segs();
    test_bsp_node_children();
    test_bsp_root_is_valid();
    printf("all tests passed\n");
    return 0;
}
