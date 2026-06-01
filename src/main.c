#include "game/game.h"
#include "mem/arena.h"
#include "ui/ui.h"
#include "wad/wad.h"
#include "raylib.h"

int main(void)
{
    Arena scratch_arena = arena_create((size_t)32 * 1024 * 1024 * 1024);

    wad_test();

    InitWindow(1280, 720, "PWDoom");
    SetTargetFPS(60);

    UIAssets ui_assets = load_ui_assets();

    GameState game;
    game_init(&game, &scratch_arena);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        game_update(&game, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game_render(&game, &scratch_arena);
        //render_ui(ui_assets, 1280, 720);

        EndDrawing();
        arena_reset(&scratch_arena);
    }

    game_shutdown(&game);
    CloseWindow();
    arena_destroy(&scratch_arena);
    unload_ui_assets(ui_assets);
    return 0;
}
