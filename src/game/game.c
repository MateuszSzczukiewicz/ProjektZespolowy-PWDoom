#include "game/game.h"

#include "mem/arena.h"
#include "raylib.h"
#include "render/bsp.h"
#include "render/render.h"

#include <assert.h>
#include <string.h>

void game_init(GameState *game, Arena *scratch)
{
    assert(game != NULL);

    memset(game, 0, sizeof(*game));
    map_init(&game->map);
    bsp_build(&game->bsp, &game->map, scratch);
    player_init(&game->player);
    player_update_camera(&game->player, &game->camera);
}

void game_update(GameState *game, float dt)
{
    assert(game != NULL);

    player_update(&game->player, (const LevelMap *)&game->map, dt);
    player_update_camera(&game->player, &game->camera);
}

void game_render(const GameState *game, Arena *scratch)
{
    assert(game != NULL);

    render_walls(&game->map, &game->bsp, &game->player, scratch);

    DrawFPS(10, 10);
}

void game_shutdown(GameState *game)
{
    assert(game != NULL);
    game->bsp.node_count = 0;
    game->bsp.seg_count = 0;
    game->bsp.leaf_count = 0;
    game->bsp.leaf_seg_count = 0;
}
