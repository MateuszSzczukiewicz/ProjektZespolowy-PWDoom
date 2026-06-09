#include "game/game.h"
#include "mem/arena.h"
#include "raylib.h"

int main(void)
{
    Arena scratch_arena = arena_create((size_t)1024 * 1024 * 1024);

    InitWindow(1280, 720, "PWDoom");
    SetTargetFPS(60);

    GameState game;
    game_init(&game, &scratch_arena);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        game_update(&game, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game_render(&game, &scratch_arena);
        EndDrawing();
        arena_reset(&scratch_arena);
    }

    game_shutdown(&game);
    CloseWindow();
    arena_destroy(&scratch_arena);
    return 0;
}
