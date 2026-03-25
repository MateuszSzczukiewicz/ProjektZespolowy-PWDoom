#ifndef PWDOOM_GAME_H
#define PWDOOM_GAME_H

#include "game/map.h"
#include "game/player.h"
#include "raylib.h"

typedef struct {
    PlayerState player;
    LevelMap map;
    Camera3D camera;
} GameState;

void game_init(GameState *game);
void game_update(GameState *game, float dt);
void game_render(const GameState *game);
void game_shutdown(GameState *game);

#endif
