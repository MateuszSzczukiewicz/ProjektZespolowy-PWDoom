#ifndef PWDOOM_COLLISION_H
#define PWDOOM_COLLISION_H

#include "game/map.h"
#include "game/player.h"
#include <stdbool.h>

bool try_move(const LevelMap *map, PlayerState *player, float dx, float dz, float dt);

#endif
