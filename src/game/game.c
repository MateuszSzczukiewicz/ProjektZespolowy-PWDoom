#include "game/game.h"

#include "mem/arena.h"
#include "raylib.h"
#include "render/bsp.h"
#include "render/render.h"
#include "wad/wad.h"

#include <assert.h>
#include <string.h>

void game_init(GameState *game, Arena *scratch)
{
    assert(game != NULL);

    memset(game, 0, sizeof(*game));

    Arena wad_arena = arena_create(8 * 1024 * 1024);
    Arena wad_scratch = arena_create(1 * 1024 * 1024);
    WadState wad = {0};
    const char *paths[] = {"assets/maps/map01.wad"};
    bool wad_ok = wad_init(paths, 1, &wad_arena, &wad_scratch, &wad);
    if (wad_ok && map_load_from_wad(&game->map, &wad, "MAP01")) {
        (void)0;
    } else {
        map_init(&game->map);
    }
    wad_free(&wad);
    arena_destroy(&wad_arena);
    arena_destroy(&wad_scratch);

    bsp_build(&game->bsp, &game->map, scratch);
    player_init(&game->player);
    player_update_camera(&game->player, &game->camera);

    game->bgm = LoadMusicStream("assets/music/Lunar_Tectonics.mp3");
    if (game->bgm.frameCount > 0) {
        PlayMusicStream(game->bgm);
        SetMusicVolume(game->bgm, 0.5f);
    }
}

void game_update(GameState *game, float dt)
{
    assert(game != NULL);

    player_update(&game->player, (const LevelMap *)&game->map, dt);
    player_update_camera(&game->player, &game->camera);

    if (game->bgm.frameCount > 0) {
        UpdateMusicStream(game->bgm);
    }

    if (IsKeyPressed(KEY_M)) {
        game->muted = !game->muted;
        if (game->muted) {
            PauseMusicStream(game->bgm);
        } else {
            ResumeMusicStream(game->bgm);
        }
    }
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
    if (game->bgm.frameCount > 0) {
        UnloadMusicStream(game->bgm);
    }
    game->bsp.node_count = 0;
    game->bsp.seg_count = 0;
    game->bsp.leaf_count = 0;
    game->bsp.leaf_seg_count = 0;
}
