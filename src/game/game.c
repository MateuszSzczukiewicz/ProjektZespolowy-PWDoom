#include "game/game.h"

#include "mem/arena.h"
#include "raylib.h"
#include "render/bsp.h"
#include "render/render.h"
#include "wad/wad.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MENU_ITEMS 2

void game_init(GameState *game, Arena *scratch)
{
    assert(game != NULL);
    (void)scratch;

    memset(game, 0, sizeof(*game));
    game->mode = MODE_MENU;
    game->menu_selection = 0;

    game->title_screen = LoadTexture("assets/title/GreedHell_title-screen_preview.jpg");

    Arena wad_arena = arena_create(8 * 1024 * 1024);
    Arena wad_scratch = arena_create(1 * 1024 * 1024);
    WadState wad = {0};
    const char *paths[] = {"assets/maps/map01.wad"};
    bool wad_ok = wad_init(paths, 1, &wad_arena, &wad_scratch, &wad);
    if (wad_ok && map_load_from_wad(&game->map, &wad, "MAP01")) {
        (void)0;
    } else {
        map_init(&game->map);
    }
    wad_free(&wad);
    arena_destroy(&wad_arena);
    arena_destroy(&wad_scratch);

    bsp_build(&game->bsp, &game->map, scratch);
    player_init(&game->player);
    if (game->map.player_start.x != 0.0f || game->map.player_start.y != 0.0f) {
        game->player.position.x = game->map.player_start.x;
        game->player.position.z = game->map.player_start.y;
        game->player.angle = game->map.player_angle * 3.14159265f / 180.0f;
    }
    player_update_camera(&game->player, &game->camera);

    game->bgm = LoadMusicStream("assets/music/Lunar_Tectonics.mp3");
    if (game->bgm.frameCount > 0) {
        PlayMusicStream(game->bgm);
        SetMusicVolume(game->bgm, 0.5f);
    }
}

static void update_menu(GameState *game)
{
    if (IsKeyPressed(KEY_DOWN)) {
        game->menu_selection++;
        if (game->menu_selection >= MENU_ITEMS)
            game->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_UP)) {
        game->menu_selection--;
        if (game->menu_selection < 0)
            game->menu_selection = MENU_ITEMS - 1;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (game->menu_selection == 0) {
            game->mode = MODE_PLAYING;
        } else {
            game->mode = MODE_PLAYING;
            game->player.position = (Vector3){0.0f, 0.0f, 0.0f};
            game->player.angle = 0.0f;
        }
    }
}

static void update_playing(GameState *game, float dt)
{
    player_update(&game->player, (const LevelMap *)&game->map, dt);
    player_update_camera(&game->player, &game->camera);

    if (IsKeyPressed(KEY_ESCAPE)) {
        game->mode = MODE_PAUSED;
    }

    if (game->bgm.frameCount > 0) {
        UpdateMusicStream(game->bgm);
    }

    if (IsKeyPressed(KEY_M)) {
        game->muted = !game->muted;
        if (game->muted) {
            PauseMusicStream(game->bgm);
        } else {
            ResumeMusicStream(game->bgm);
        }
    }
}

static void update_paused(GameState *game)
{
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->mode = MODE_PLAYING;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        game->menu_selection++;
        if (game->menu_selection >= MENU_ITEMS)
            game->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_UP)) {
        game->menu_selection--;
        if (game->menu_selection < 0)
            game->menu_selection = MENU_ITEMS - 1;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (game->menu_selection == 0) {
            game->mode = MODE_PLAYING;
        } else {
            game->mode = MODE_MENU;
        }
    }
}

void game_update(GameState *game, float dt)
{
    assert(game != NULL);

    if (game->mode == MODE_MENU) {
        update_menu(game);
    } else if (game->mode == MODE_PLAYING) {
        update_playing(game, dt);
    } else if (game->mode == MODE_PAUSED) {
        update_paused(game);
    }
}

static void draw_menu_background(const GameState *game)
{
    if (game->title_screen.id > 0) {
        DrawTexture(game->title_screen, 0, 0, WHITE);
    } else {
        ClearBackground(BLACK);
    }
}

static void draw_menu_overlay(const GameState *game)
{
    const char *items[MENU_ITEMS] = {"NEW GAME", "EXIT"};
    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();
    int y_start = screen_h / 2;
    int spacing = 40;
    int font_size = 20;

    for (int i = 0; i < MENU_ITEMS; i++) {
        Color color = (i == game->menu_selection) ? RED : WHITE;
        int text_w = MeasureText(items[i], font_size);
        int x = (screen_w - text_w) / 2;
        int y = y_start + i * spacing;
        DrawText(items[i], x, y, font_size, color);
    }
}

static void draw_pause_overlay(const GameState *game)
{
    const char *items[MENU_ITEMS] = {"RESUME", "QUIT TO MENU"};
    int screen_w = GetScreenWidth();
    int screen_h = GetScreenHeight();
    int y_start = screen_h / 2;
    int spacing = 40;
    int font_size = 20;

    DrawRectangle(0, 0, screen_w, screen_h, (Color){0, 0, 0, 128});

    for (int i = 0; i < MENU_ITEMS; i++) {
        Color color = (i == game->menu_selection) ? RED : WHITE;
        int text_w = MeasureText(items[i], font_size);
        int x = (screen_w - text_w) / 2;
        int y = y_start + i * spacing;
        DrawText(items[i], x, y, font_size, color);
    }
}

void game_render(const GameState *game, Arena *scratch)
{
    assert(game != NULL);

    if (game->mode == MODE_MENU) {
        draw_menu_background(game);
        draw_menu_overlay(game);
    } else if (game->mode == MODE_PLAYING) {
        render_walls(&game->map, &game->bsp, &game->player, scratch);
        DrawFPS(10, 10);
    } else if (game->mode == MODE_PAUSED) {
        render_walls(&game->map, &game->bsp, &game->player, scratch);
        draw_pause_overlay(game);
    }
}

void game_shutdown(GameState *game)
{
    assert(game != NULL);
    if (game->bgm.frameCount > 0) {
        UnloadMusicStream(game->bgm);
    }
    if (game->title_screen.id > 0) {
        UnloadTexture(game->title_screen);
    }
    game->bsp.node_count = 0;
    game->bsp.seg_count = 0;
    game->bsp.leaf_count = 0;
    game->bsp.leaf_seg_count = 0;
}
