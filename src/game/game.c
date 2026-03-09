#include "game/game.h"

void game_init(Game *game)
{
    game->player_pos = (Vector2){640.0f, 360.0f};
}

void game_update(Game *game, float dt)
{
    (void)game;
    (void)dt;
    // TODO: input handling, physics, game logic
}

void game_render(const Game *game)
{
    DrawText("PWDoom", 560, 300, 40, RED);
    DrawCircleV(game->player_pos, 10.0f, YELLOW);
}

void game_shutdown(Game *game)
{
    (void)game;
}
