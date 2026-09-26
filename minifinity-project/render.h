#ifndef RENDER_H
#define RENDER_H
#include "gl.h"
#include "game.h"
#define RES_WIDTH  1920
#define RES_HEIGHT 1080
#define COLOR_CURSOR GL_AMBER

typedef struct {
    int x, y;
    int width, height;
    int nrows, ncols;
} textbox;

typedef struct {
    int x, y;
    int width, height;
    color_t bgcolor;
        textbox tb;
    int tb_top;
} frame;

static struct {
    frame f1, f2;
} display;

void render_init(int nwords);
int render_field_size();
void render_start(game_state_t* game);
void render(game_state_t* game);
void render_cursor_only(game_state_t* game);
void render_text(textbox* tb, player_t* player);
void render_cursor_anim(game_state_t* game);
void render_stats_only(game_state_t* game);
void render_step_cursors(game_state_t* game);
void render_message(int player_num, const char* msg);
void render_menu(game_state_t* game, select active);
void render_countdown();
void render_celebrate(game_state_t* game, int player_num, keyboard_t* kb1, keyboard_t* kb2);
void render_finish_overlay(game_state_t* game, int player_num);
void render_finish_stats(game_state_t* game, int player_num);
void render_text_rows(textbox* tb, player_t* player, int min_row, int max_row, color_t bgcolor, game_state_t* game);
void render_word(textbox* tb, player_t* player, int target_word_idx, color_t bgcolor, game_state_t* game);
void render_stats(game_state_t* game);
void redraw_char_at_pixel(textbox* tb, player_t* player, int px, int py, color_t bgcolor, game_state_t* game);
void render_char_at(textbox* tb, player_t* player, int idx, color_t bgcolor, game_state_t* game);
void render_replay();

#endif