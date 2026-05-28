#ifndef PWDOOM_RENDER_H
#define PWDOOM_RENDER_H

#include "game/map.h"
#include "game/player.h"
#include "mem/arena.h"
#include "render/bsp.h"

void render_walls(const LevelMap *map, const BSPTree *bsp,
                  const PlayerState *player, Arena *scratch);

#endif
