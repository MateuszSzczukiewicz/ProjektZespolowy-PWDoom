#ifndef PWDOOM_PLAYER_H
#define PWDOOM_PLAYER_H

#include "game/map.h"
#include "raylib.h"

typedef struct {
    Vector3 position;
    float angle;
    float speed;
} PlayerState;

void player_init(PlayerState *player);

void player_update(PlayerState *player, const LevelMap *map, float dt);

void player_update_camera(const PlayerState *player, Camera3D *camera);

#endif