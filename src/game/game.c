#include "game/game.h"
#include "render/render.h"

#include "raylib.h"

#include <assert.h>
#include <string.h>

void game_init(GameState *game)
{
    assert(game != NULL);
    memset(game, 0, sizeof(*game));

    map_init(&game->map);
    player_init(&game->player);
    player_update_camera(&game->player, &game->camera);
}

void game_update(GameState *game, float dt)
{
    assert(game != NULL);

    player_update(&game->player, (const LevelMap *)&game->map, dt);
    player_update_camera(&game->player, &game->camera);
}

void game_render(const GameState *game)
{
    assert(game != NULL);

    render_walls(&game->map, &game->player);

    DrawFPS(10, 10);
}

void game_shutdown(GameState *game)
{
    assert(game != NULL);
    (void)game;
}
