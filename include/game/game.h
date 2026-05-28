#ifndef PWDOOM_GAME_H
#define PWDOOM_GAME_H

#include "raylib.h"

typedef struct {
    Vector2 player_pos;
} Game;

void game_init(Game *game);
void game_update(Game *game, float dt);
void game_render(const Game *game);
void game_shutdown(Game *game);

#endif
