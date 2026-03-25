#include "game/player.h"

#include "game/collision.h"
#include "raylib.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>

#define PLAYER_MOVE_SPEED 15.0f
#define PLAYER_TURN_SPEED 2.5f
#define PLAYER_EYE_HEIGHT 0.4f
#define PLAYER_FRICTION   0.85f

void player_init(PlayerState *player)
{
    assert(player != NULL);

    player->position = (Vector3){1.5f, PLAYER_EYE_HEIGHT, 1.5f};
    player->angle = 0.0f;
    player->speed = PLAYER_MOVE_SPEED;
    player->dx = 0.0f;
    player->dz = 0.0f;
    player->view_z = PLAYER_EYE_HEIGHT;
    player->floor_z = 0.0f;
    player->sector_id = 0;
}

void player_update(PlayerState *player, const LevelMap *map, float dt)
{
    assert(player != NULL);
    assert(map != NULL);

    if (IsKeyDown(KEY_LEFT)) {
        player->angle -= PLAYER_TURN_SPEED * dt;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        player->angle += PLAYER_TURN_SPEED * dt;
    }

    float dir_x = sinf(player->angle);
    float dir_z = cosf(player->angle);

    float move_x = 0.0f;
    float move_z = 0.0f;

    if (IsKeyDown(KEY_W)) {
        move_x += dir_x;
        move_z += dir_z;
    }
    if (IsKeyDown(KEY_S)) {
        move_x -= dir_x;
        move_z -= dir_z;
    }
    if (IsKeyDown(KEY_A)) {
        move_x -= dir_z;
        move_z += dir_x;
    }
    if (IsKeyDown(KEY_D)) {
        move_x += dir_z;
        move_z -= dir_x;
    }

    float accel = player->speed * dt;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        accel *= 2.0f;
    }

    float len = sqrtf(move_x * move_x + move_z * move_z);
    if (len > 0.0f) {
        move_x = (move_x / len) * accel;
        move_z = (move_z / len) * accel;
    }

    player->dx += move_x;
    player->dz += move_z;

    float friction = powf(PLAYER_FRICTION, dt * 60.0f);
    player->dx *= friction;
    player->dz *= friction;

    try_move(map, player, player->dx, player->dz, dt);

    player->floor_z = get_floor_height(map, player->position.x, player->position.z);
    player->view_z = player->floor_z + PLAYER_EYE_HEIGHT;
    player->position.y = player->view_z;
}

void player_update_camera(const PlayerState *player, Camera3D *camera)
{
    assert(player != NULL);
    assert(camera != NULL);

    float dir_x = sinf(player->angle);
    float dir_z = cosf(player->angle);

    camera->position = player->position;
    camera->target = (Vector3){
        player->position.x + dir_x,
        player->position.y,
        player->position.z + dir_z,
    };
    camera->up = (Vector3){0.0f, 1.0f, 0.0f};
    camera->fovy = 90.0f;
    camera->projection = CAMERA_PERSPECTIVE;
}
