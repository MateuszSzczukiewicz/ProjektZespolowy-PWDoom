#ifndef PWDOOM_GAME_H
#define PWDOOM_GAME_H

#include "game/map.h"
#include "game/player.h"
#include "mem/arena.h"
#include "raylib.h"
#include "render/bsp.h"

#include <stdbool.h>

typedef enum { MODE_MENU, MODE_PLAYING, MODE_PAUSED } GameMode;

typedef struct {
    PlayerState player;
    LevelMap map;
    BSPTree bsp;
    Camera3D camera;
    Music bgm;
    bool muted;
    GameMode mode;
    int menu_selection;
    Texture2D title_screen;
} GameState;

void game_init(GameState *game, Arena *scratch);
void game_update(GameState *game, float dt);
void game_render(const GameState *game, Arena *scratch);
void game_shutdown(GameState *game);

#endif
