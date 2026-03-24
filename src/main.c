#include "game/game.h"
#include "raylib.h"

int main(void)
{
    t_arena scratch_arena = arena_create((size_t)(1024)*1024*1024*32);

    InitWindow(1280, 720, "PWDoom");
    SetTargetFPS(60);

    Game game;
    game_init(&game);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        game_update(&game, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game_render(&game);
        EndDrawing();
        arena_reset(&scratch_arena);
    }

    game_shutdown(&game);
    CloseWindow();
    arena_destroy(&scratch_arena);
    return 0;
}
