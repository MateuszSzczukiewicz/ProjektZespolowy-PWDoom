#include "game/collision.h"
#include "game/map.h"
#include "game/player.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void test_map_init(void)
{
    LevelMap map;
    map_init(&map);

    assert(map.vertex_count == 12);
    assert(map.sector_count == 3);
    assert(map.sidedef_count == 3);
    assert(map.linedef_count == 14);

    assert(map.vertices[0].x == 0.0f);
    assert(map.vertices[0].y == 0.0f);

    assert(map.sectors[0].floor_height == 0.0f);
    assert(map.sectors[0].ceiling_height == 3.0f);
    assert(map.sectors[1].floor_height == 0.5f);

    printf("  map_init: OK\n");
}

static void test_floor_height(void)
{
    LevelMap map;
    map_init(&map);

    float h = get_floor_height(&map, 1.5f, 1.5f);
    assert(h == 0.0f);

    float h2 = get_floor_height(&map, 6.0f, 2.5f);
    assert(h2 == 0.5f);

    printf("  get_floor_height: OK\n");
}

static void test_collision_blocks_outside(void)
{
    LevelMap map;
    map_init(&map);

    PlayerState player;
    memset(&player, 0, sizeof(player));
    player.position.x = 2.0f;
    player.position.z = 2.5f;
    player.floor_z = 0.0f;

    for (int i = 0; i < 120; i++) {
        player.dx = -5.0f;
        player.dz = 0.0f;
        try_move(&map, &player, player.dx, player.dz, 1.0f / 60.0f);
    }

    assert(player.position.x > 0.0f);

    printf("  collision (wall block): OK\n");
}

static void test_collision_allows_inside(void)
{
    LevelMap map;
    map_init(&map);

    PlayerState player;
    memset(&player, 0, sizeof(player));
    player.position.x = 2.0f;
    player.position.z = 2.0f;
    player.floor_z = 0.0f;

    float old_x = player.position.x;
    try_move(&map, &player, 0.5f, 0.0f, 1.0f);

    assert(fabsf(player.position.x - (old_x + 0.5f)) < 0.01f);

    printf("  collision (free move): OK\n");
}

int main(void)
{
    printf("PWDoom smoke tests\n");
    test_map_init();
    test_floor_height();
    test_collision_blocks_outside();
    test_collision_allows_inside();
    printf("all tests passed\n");
    return 0;
}
