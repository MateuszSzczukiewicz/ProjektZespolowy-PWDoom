#ifndef PWDOOM_MAP_H
#define PWDOOM_MAP_H

#include <stdbool.h>

#define MAP_WIDTH 10
#define MAP_HEIGHT 10

typedef int LevelMap[MAP_HEIGHT][MAP_WIDTH];

bool can_move(const LevelMap *map, float x, float z);

void map_init(LevelMap map);

void map_render(const LevelMap *map);

#endif