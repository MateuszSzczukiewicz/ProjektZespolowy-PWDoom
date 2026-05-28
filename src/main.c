#include "game/game.h"
#include "raylib.h"

int main(void)
{
    InitWindow(1280, 720, "PWDoom");
    SetTargetFPS(60);

    GameState game;
    game_init(&game);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        game_update(&game, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game_render(&game);
        EndDrawing();
    }

    game_shutdown(&game);
    CloseWindow();
    return 0;
}
