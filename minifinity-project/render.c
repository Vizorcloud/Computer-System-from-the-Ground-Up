/* File: render.c
 * --------------
 * render.c implements the UI for MangoType
 */

#include <stdbool.h>
#include "gl.h"
#include "game.h"
#include "render.h"
#include "strings.h"
#include "malloc.h"
#include "stats.h"
#include "timer.h"

#define RES_WIDTH  1920
#define RES_HEIGHT 1080
#define TB_LR      80

#define COLOR_P1_BG       0xFF1F0F0E
#define COLOR_P2_BG       0xFF130E1F
#define COLOR_N_BG        0xFF190E16
#define COLOR_UNTYPED     GL_SILVER
#define COLOR_RIGHT       GL_WHITE
#define COLOR_WRONG       GL_RED
#define COLOR_UNTYPED_GREEN 0xFF006400
#define COLOR_RIGHT_GREEN   0xFF00FF00
#define COLOR_SELECT      0xFFfad641
#define COLOR_CURSOR      GL_AMBER

#define LINE_SPACING 10
#define CURSOR_SPEED 12
#define TB_BORDER    2
#define TB_PAD       8

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

static color_t acctocolor(result res) {
    switch (res) {
        case RIGHT:  return COLOR_RIGHT;
        case WRONG:  return COLOR_WRONG;
        default:     return COLOR_UNTYPED;
    }
}

static bool is_bonus_word(game_state_t *game, int word_idx) {
    for (int i = 0; i < game->num_bonus_words; i++) {
        if (game->bonus_word_idxs[i] == word_idx && !game->bonus_word_claimed[i])
            return true;
    }
    return false;
}

static color_t rainbow_color(int char_idx) {
    static color_t colors[] = {
        GL_RED, 0xFFFF7700, GL_YELLOW, GL_GREEN, GL_CYAN, 0xFF0000FF, GL_MAGENTA
    };
    unsigned int tick = timer_get_ticks() / (TICKS_PER_USEC * 100000);
    return colors[(char_idx + tick) % 7];
}

static color_t camo_color(color_t bgcolor) {
    unsigned int r = (bgcolor >> 16) & 0xFF;
    unsigned int g = (bgcolor >>  8) & 0xFF;
    unsigned int b = (bgcolor)       & 0xFF;
    r = (r + 30 > 255) ? 255 : r + 30;
    g = (g + 30 > 255) ? 255 : g + 30;
    b = (b + 30 > 255) ? 255 : b + 30;
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

static color_t get_char_color(game_state_t *game, player_t *player,
                               int word_idx, int char_idx,
                               int player_num, color_t bgcolor) {
    if (player->active_powerup == POWERUP_RAINBOW)
        return rainbow_color(char_idx);

    bool unclaimed_bonus = is_bonus_word(game, word_idx);
    bool claimed_bonus = false;
    int claimer = 0;
    for (int b = 0; b < game->num_bonus_words; b++) {
        if (game->bonus_word_idxs[b] == word_idx && game->bonus_word_claimed[b]) {
            claimed_bonus = true;
            claimer = game->bonus_word_claimer[b];
            break;
        }
    }

    if (player->active_powerup == POWERUP_CAMO)
        return camo_color(bgcolor);

    if (unclaimed_bonus || (claimed_bonus && claimer == player_num)) {
        return (player->acc[char_idx] == RIGHT) ? GL_GREEN :
               (player->acc[char_idx] == WRONG) ? COLOR_WRONG : COLOR_UNTYPED_GREEN;
    }
    return acctocolor(player->acc[char_idx]);
}

static void int_to_str(int n, char *buf) {
    if (n <= 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    int i = 0;
    char tmp[12];
    while (n > 0) { tmp[i++] = '0' + (n % 10); n /= 10; }
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
}

static void build_stats_str(char *buf, int acc, int wpm, int countdown) {
    const char *prefix = "Accuracy: ";
    const char *mid    = "% | WPM: ";
    const char *mid2   = " | Time: ";
    const char *suffix = "s";
    int i = 0;
    while (prefix[i]) { buf[i] = prefix[i]; i++; }
    char num[12];
    int_to_str(acc, num);
    int j = 0;
    while (num[j]) buf[i++] = num[j++];
    j = 0; while (mid[j])  buf[i++] = mid[j++];
    int_to_str(wpm, num);
    j = 0; while (num[j])  buf[i++] = num[j++];
    j = 0; while (mid2[j]) buf[i++] = mid2[j++];
    int_to_str(countdown, num);
    j = 0; while (num[j])    buf[i++] = num[j++];
    j = 0; while (suffix[j]) buf[i++] = suffix[j++];
    buf[i] = '\0';
}

static int get_countdown(game_state_t *game) {
    if (game->start_time == 0) return game->duration_secs;
    unsigned int elapsed = (timer_get_ticks() - game->start_time) / (TICKS_PER_USEC * 1000000);
    int remaining = game->duration_secs - (int)elapsed;
    return remaining < 0 ? 0 : remaining;
}

void render_message(int player_num, const char *msg) {
    static unsigned int last_msg_tick[2] = {0, 0};
    static color_t last_color[2] = {GL_AMBER, GL_AMBER};
    int idx = player_num - 1;

    frame *f = (player_num == 1) ? &display.f1 : &display.f2;
    int chh = gl_get_char_height();
    int chw = gl_get_char_width();
    int padding = 20;
    int box_h = chh + 2 * padding;
    int box_y = f->y + RES_HEIGHT - box_h - padding;

    unsigned int tick = timer_get_ticks() / (TICKS_PER_USEC * 100000);
    if (tick != last_msg_tick[idx]) {
        static color_t colors[] = {GL_AMBER, GL_YELLOW, GL_GREEN, GL_CYAN, GL_MAGENTA};
        last_color[idx] = colors[tick % 5];
        last_msg_tick[idx] = tick;
    } else {
        return;
    }

    int msg_x = f->x + (f->width - (int)strlen(msg) * chw) / 2;
    gl_draw_rect(f->x + padding, box_y, f->width - 2 * padding, box_h, f->bgcolor);
    gl_draw_string(msg_x, box_y + padding, msg, last_color[idx]);
}

void render_stats(game_state_t *game) {
    frame *f1 = &display.f1;
    frame *f2 = &display.f2;
    int chh = gl_get_char_height();
    int chw = gl_get_char_width();
    int title_y = f1->tb_top / 2 - chh / 2;
    int stats_y = title_y + chh + 12;
    int countdown = get_countdown(game);

    char stats1[64], stats2[64];
    build_stats_str(stats1, player_get_accuracy(&game->p1), player_get_wpm(&game->p1), countdown);
    build_stats_str(stats2, player_get_accuracy(&game->p2), player_get_wpm(&game->p2), countdown);

    if (!game->p1_screen_done) {
        gl_draw_rect(f1->x, stats_y, f1->width, chh, f1->bgcolor);
        gl_draw_string(f1->x + (f1->width - (int)strlen(stats1) * chw) / 2, stats_y, stats1, GL_AMBER);
    }
    if (!game->p2_screen_done) {
        gl_draw_rect(f2->x, stats_y, f2->width, chh, f2->bgcolor);
        gl_draw_string(f2->x + (f2->width - (int)strlen(stats2) * chw) / 2, stats_y, stats2, GL_AMBER);
    }
}

static void gl_draw_char_flipped(int x, int y, char ch, color_t c, color_t bgcolor) {
    int w = gl_get_char_width();
    int h = gl_get_char_height();
    int tx = RES_WIDTH  - w - 1;
    int ty = RES_HEIGHT - h - 1;
    gl_draw_rect(tx, ty, w, h, bgcolor);
    gl_draw_char(tx, ty, ch, c);
    gl_draw_rect(x, y, w, h, bgcolor);
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            color_t pixel = gl_read_pixel(tx + col, ty + row);
            if (pixel != bgcolor)
                gl_draw_pixel(x + (w - 1 - col), y + (h - 1 - row), pixel);
        }
    }
    gl_draw_rect(tx, ty, w, h, bgcolor);
}

static void draw_char_with_powerup(int x, int y, char ch, color_t c,
                                    color_t bgcolor, player_t *player) {
    if (player->active_powerup == POWERUP_FLIP)
        gl_draw_char_flipped(x, y, ch, c, bgcolor);
    else
        gl_draw_char(x, y, ch, c);
}

void get_cursor_target(textbox *tb, player_t *player, int *out_x, int *out_y) {
    int n   = player->promptlen;
    int chw = gl_get_char_width();
    int chh = gl_get_char_height() + LINE_SPACING;
    int row = 0, col = 0, i = 0;

    while (i < n) {
        int wlen = 0;
        while (i + wlen < n && player->promptstr[i + wlen] != ' ') wlen++;
        if (col + wlen + 1 > tb->ncols) { col = 0; row++; }
        if (row >= tb->nrows) break;

        for (int j = 0; j < wlen; j++) {
            if (i == player->cursor) {
                *out_x = tb->x + col * chw;
                *out_y = tb->y + row * chh;
                return;
            }
            col++; i++;
        }
        if (i < n && player->promptstr[i] == ' ') {
            if (i == player->cursor) {
                *out_x = tb->x + col * chw;
                *out_y = tb->y + row * chh;
                return;
            }
            col++; i++;
            if (col >= tb->ncols) { col = 0; row++; }
        }
    }
    *out_x = tb->x;
    *out_y = tb->y;
}

void redraw_char_at_pixel(textbox *tb, player_t *player, int px, int py, color_t bgcolor, game_state_t *game) {
    if (player->active_powerup == POWERUP_FLIP) return;
    int player_num = (player == &game->p1) ? 1 : 2;
    int chw = gl_get_char_width();
    int chh = gl_get_char_height() + LINE_SPACING;
    int n   = player->promptlen;
    int row = 0, col = 0, i = 0, word_idx = 0;

    while (i < n) {
        int wlen = 0;
        while (i + wlen < n && player->promptstr[i + wlen] != ' ') wlen++;
        if (col + wlen + 1 > tb->ncols) { col = 0; row++; }
        if (row >= tb->nrows) break;

        for (int j = 0; j < wlen; j++) {
            int x = tb->x + col * chw;
            int y = tb->y + row * chh;
            if (px >= x && px < x + chw && py >= y && py < y + chh) {
                color_t c = get_char_color(game, player, word_idx, i, player_num, bgcolor);
                gl_draw_rect(x, y, chw, gl_get_char_height(), bgcolor);
                draw_char_with_powerup(x, y, player->promptstr[i], c, bgcolor, player);
                return;
            }
            col++; i++;
        }
        if (i < n && player->promptstr[i] == ' ') {
            col++; i++;
            if (col >= tb->ncols) { col = 0; row++; }
        }
        word_idx++;
    }
}

void render_char_at(textbox *tb, player_t *player, int idx, color_t bgcolor, game_state_t *game) {
    if (player->active_powerup == POWERUP_FLIP) return;
    int player_num = (player == &game->p1) ? 1 : 2;
    int n   = player->promptlen;
    int chw = gl_get_char_width();
    int chh = gl_get_char_height() + LINE_SPACING;
    int row = 0, col = 0, i = 0, word_idx = 0;

    while (i < n) {
        int wlen = 0;
        while (i + wlen < n && player->promptstr[i + wlen] != ' ') wlen++;
        if (col + wlen + 1 > tb->ncols) { col = 0; row++; }
        if (row >= tb->nrows) break;

        for (int j = 0; j < wlen; j++) {
            if (i == idx) {
                int x = tb->x + col * chw;
                int y = tb->y + row * chh;
                color_t c = get_char_color(game, player, word_idx, i, player_num, bgcolor);
                gl_draw_rect(x, y, chw, chh, bgcolor);
                draw_char_with_powerup(x, y, player->promptstr[i], c, bgcolor, player);
                return;
            }
            col++; i++;
        }
        if (i < n && player->promptstr[i] == ' ') {
            col++; i++;
            if (col >= tb->ncols) { col = 0; row++; }
        }
        word_idx++;
    }
}

void step_cursor(player_t *player, int target_x, int target_y) {
    if (player->cursor_y != target_y) {
        player->cursor_x = target_x;
        player->cursor_y = target_y;
        return;
    }
    int diff = target_x - player->cursor_x;
    if      (diff == 0)              {}
    else if (diff > -2 && diff < 2)  player->cursor_x = target_x;
    else                             player->cursor_x += diff / 2 + (diff > 0 ? 1 : -1);
    player->cursor_y = target_y;
}

static void render_cursor(textbox *tb, player_t *player) {
    if (player->cursor >= player->promptlen) return;
    int tx, ty;
    get_cursor_target(tb, player, &tx, &ty);
    step_cursor(player, tx, ty);
    gl_draw_rect(player->cursor_x, player->cursor_y, 2, gl_get_char_height(), COLOR_CURSOR);
}

void render_cursor_only(game_state_t *game) {
    frame *f1 = &display.f1;
    frame *f2 = &display.f2;
    int ox1 = game->p1.cursor_x, oy1 = game->p1.cursor_y;
    int ox2 = game->p2.cursor_x, oy2 = game->p2.cursor_y;

    for (int i = 0; i < 2; i++) {
        gl_draw_rect(ox1, oy1, 2, gl_get_char_height(), f1->bgcolor);
        gl_draw_rect(ox2, oy2, 2, gl_get_char_height(), f2->bgcolor);
        render_cursor(&f1->tb, &game->p1);
        render_cursor(&f2->tb, &game->p2);
        gl_swap_buffer();
    }
}

void render_init(int nwords) { gl_init(RES_WIDTH, RES_HEIGHT, GL_DOUBLEBUFFER);

    int chh     = gl_get_char_height() + LINE_SPACING;
    int chw     = gl_get_char_width();
    int tb_width = RES_WIDTH / 2 - 2 * TB_LR;
    int ncols   = tb_width / chw;
    int nrows   = (nwords * 6) / ncols + 1;
    int tb_top  = (RES_HEIGHT - nrows * chh) / 2;

    frame *f1 = &display.f1;
    f1->x = 0; f1->y = 0;
    f1->width = RES_WIDTH / 2; f1->height = RES_HEIGHT;
    f1->bgcolor = COLOR_P1_BG;
    f1->tb.x = f1->x + TB_LR;  f1->tb.y = f1->y + tb_top;
    f1->tb.width = tb_width;    f1->tb.height = RES_HEIGHT - tb_top;
    f1->tb.ncols = ncols;       f1->tb.nrows  = f1->tb.height / chh;
    f1->tb_top = tb_top;

    frame *f2 = &display.f2;
    f2->x = RES_WIDTH / 2; f2->y = 0;
    f2->width = RES_WIDTH / 2; f2->height = RES_HEIGHT;
    f2->bgcolor = COLOR_P2_BG;
    f2->tb.x = f2->x + TB_LR;  f2->tb.y = f2->y + tb_top;
    f2->tb.width = tb_width;    f2->tb.height = RES_HEIGHT - tb_top;
    f2->tb.ncols = ncols;       f2->tb.nrows  = f2->tb.height / chh;
    f2->tb_top = tb_top;
}

int render_field_size(void) {
    return display.f1.tb.ncols * display.f1.tb.nrows;
}

static void render_prompt(player_t *player, textbox *tb, const char *words[], int nwords, game_state_t *game) {
    int total = 0;
    for (int i = 0; i < nwords; i++) total += strlen(words[i]) + 1;
    player->promptstr = malloc(total);
    player->promptlen = total - 1;

    char *p = player->promptstr;
    for (int i = 0; i < nwords; i++) {
        int len = strlen(words[i]);
        memcpy(p, words[i], len);
        p += len;
        *p++ = (i < nwords - 1) ? ' ' : '\0';
    }

    player->acc = malloc(total * sizeof(result));
    memset(player->acc, UNTYPED, total * sizeof(result));

    int chw = gl_get_char_width();
    int chh = gl_get_char_height() + LINE_SPACING;
    int row = 0, col = 0;
    for (int i = 0; i < nwords; i++) {
        int len = strlen(words[i]);
        if (col + len + 1 > tb->ncols) { col = 0; row++; }
        if (row >= tb->nrows) break;
        color_t wc = is_bonus_word(game, i) ? COLOR_UNTYPED_GREEN : COLOR_UNTYPED;
        gl_draw_string(tb->x + col * chw, tb->y + row * chh, words[i], wc);
        col += len + 1;
    }
}

static void render_titles(game_state_t *game) {
    frame *f1 = &display.f1;
    frame *f2 = &display.f2;
    int chh = gl_get_char_height();
    int chw = gl_get_char_width();
    int ty  = f1->tb_top / 2 - chh / 2;
    gl_draw_string(f1->x + (f1->width - (int)strlen(game->p1.name) * chw) / 2, ty, game->p1.name, GL_WHITE);
    gl_draw_string(f2->x + (f2->width - (int)strlen(game->p2.name) * chw) / 2, ty, game->p2.name, GL_WHITE);
}

typedef struct { int x, y; } position;

static position midtextpos(const char *text) {
    int w = gl_get_char_width()  * (int)strlen(text);
    int h = gl_get_char_height();
    return (position){ (RES_WIDTH - w) / 2, (RES_HEIGHT - h) / 2 };
}

static void countdown(const char *sec, position p) {
    for (int i = 0; i < 2; i++) {
        gl_clear(GL_BLACK);
        gl_draw_string(p.x, p.y, sec, GL_WHITE);
        gl_swap_buffer();
    }
    timer_delay(1);
}

void render_countdown(void) {
    gl_clear(GL_BLACK);
    gl_swap_buffer();
    timer_delay_ms(500);
    position p = midtextpos("3");
    countdown("3", p);
    countdown("2", p);
    countdown("1", p);
    p = midtextpos("GO!");
    countdown("GO!", p);
}

void render_text_rows(textbox *tb, player_t *player, int min_row, int max_row, color_t bgcolor, game_state_t *game) {
    int player_num = (player == &game->p1) ? 1 : 2;
    int n   = player->promptlen;
    int chw = gl_get_char_width();
    int chh = gl_get_char_height() + LINE_SPACING;
    int row = 0, col = 0, i = 0, word_idx = 0;

    while (i < n) {
        int wlen = 0;
        while (i + wlen < n && player->promptstr[i + wlen] != ' ') wlen++;
        if (col + wlen + 1 > tb->ncols) { col = 0; row++; }
        if (row >= tb->nrows) break;

        for (int j = 0; j < wlen; j++) {
            if (row >= min_row && row <= max_row) {
                color_t c = get_char_color(game, player, word_idx, i, player_num, bgcolor);
                draw_char_with_powerup(tb->x + col * chw, tb->y + row * chh,
                                       player->promptstr[i], c, bgcolor, player);
            }
            col++; i++;
        }
        if (i < n && player->promptstr[i] == ' ') {
            col++; i++;
            if (col >= tb->ncols) { col = 0; row++; }
        }
        word_idx++;
    }
}

void render_start(game_state_t *game) {
    frame *f1 = &display.f1;
    frame *f2 = &display.f2;

    render_prompt(&game->p1, &f1->tb, game->p1.prompt, game->nwords, game);
    render_prompt(&game->p2, &f2->tb, game->p2.prompt, game->nwords, game);

    for (int i = 0; i < 2; i++) {
        gl_draw_rect(f1->x, f1->y, f1->width, f1->height, f1->bgcolor);
        gl_draw_rect(f2->x, f2->y, f2->width, f2->height, f2->bgcolor);
        render_text_rows(&f1->tb, &game->p1, 0, 999, f1->bgcolor, game);
        render_text_rows(&f2->tb, &game->p2, 0, 999, f2->bgcolor, game);
        render_titles(game);
        render_stats(game);
        gl_swap_buffer();
    }
    for (int i = 0; i < 2; i++) {
        render_cursor(&f1->tb, &game->p1);
        render_cursor(&f2->tb, &game->p2);
        render_stats(game);
        gl_swap_buffer();
    }
}

void render_step_cursors(game_state_t *game) {
    int tx1, ty1, tx2, ty2;
    get_cursor_target(&display.f1.tb, &game->p1, &tx1, &ty1);
    get_cursor_target(&display.f2.tb, &game->p2, &tx2, &ty2);
    step_cursor(&game->p1, tx1, ty1);
    step_cursor(&game->p2, tx2, ty2);
}

static void int_to_str_padded(int n, char *buf, int width) {
    char tmp[12];
    int len = 0;
    if (n == 0) { tmp[len++] = '0'; }
    else { while (n > 0) { tmp[len++] = '0' + (n % 10); n /= 10; } }
    int pad = width - len;
    int i = 0;
    while (pad-- > 0) buf[i++] = '0';
    while (len > 0)   buf[i++] = tmp[--len];
    buf[i] = '\0';
}

void render_celebrate(game_state_t *game, int player_num, keyboard_t *kb1, keyboard_t *kb2) {
    frame    *f = (player_num == 1) ? &display.f1 : &display.f2;
    player_t *p = (player_num == 1) ? &game->p1   : &game->p2;
    int chh      = gl_get_char_height();
    int chw      = gl_get_char_width();
    int center_x = f->x + f->width / 2;
    int line_h   = chh + LINE_SPACING * 2;
    int base_y   = f->y + f->height / 2 - 5 * line_h;

    int acc        = p->final_acc;
    int wpm        = p->final_wpm;
    int time_bonus = p->final_time_bonus;
    int bonus_pts  = p->bonus_score * 100;
    int score      = p->final_score;

    char buf[12];
    char acc_str[32];   int acc_len   = 0;
    char wpm_str[32];   int wpm_len   = 0;
    char time_str[32];  int time_len  = 0;
    char bonus_str[32]; int bonus_len = 0;
    char score_str[32]; int score_len = 0;

    const char *acc_label   = "Accuracy: ";
    const char *wpm_label   = "WPM:      ";
    const char *time_label  = "Time:    +";
    const char *bonus_label = "Bonus:   +";
    const char *score_label = "Score:      ";

    for (int j = 0; acc_label[j];   j++) acc_str[acc_len++]     = acc_label[j];
    int_to_str_padded(acc, buf, 6);
    for (int j = 0; buf[j]; j++) acc_str[acc_len++] = buf[j];
    acc_str[acc_len++] = '%'; acc_str[acc_len] = '\0';

    for (int j = 0; wpm_label[j];   j++) wpm_str[wpm_len++]     = wpm_label[j];
    int_to_str_padded(wpm, buf, 6);
    for (int j = 0; buf[j]; j++) wpm_str[wpm_len++] = buf[j];
    wpm_str[wpm_len] = '\0';

    for (int j = 0; time_label[j];  j++) time_str[time_len++]   = time_label[j];
    int_to_str_padded(time_bonus, buf, 6);
    for (int j = 0; buf[j]; j++) time_str[time_len++] = buf[j];
    time_str[time_len] = '\0';

    for (int j = 0; bonus_label[j]; j++) bonus_str[bonus_len++] = bonus_label[j];
    int_to_str_padded(bonus_pts, buf, 6);
    for (int j = 0; buf[j]; j++) bonus_str[bonus_len++] = buf[j];
    bonus_str[bonus_len] = '\0';

    for (int j = 0; score_label[j]; j++) score_str[score_len++] = score_label[j];
    int_to_str_padded(score, buf, 6);
    for (int j = 0; buf[j]; j++) score_str[score_len++] = buf[j];
    score_str[score_len] = '\0';

    char acc_display[40];   int ad = 0;
    char wpm_display[40];   int wd = 0;
    char time_display[40];  int td = 0;
    char bonus_display[40]; int bd = 0;

    acc_display[ad++]   = 'x'; acc_display[ad++]   = ' ';
    wpm_display[wd++]   = 'x'; wpm_display[wd++]   = ' ';
    time_display[td++]  = '+'; time_display[td++]  = ' ';
    bonus_display[bd++] = '+'; bonus_display[bd++] = ' ';

    for (int j = 0; j < acc_len;   j++) acc_display[ad++]   = acc_str[j];
    for (int j = 0; j < wpm_len;   j++) wpm_display[wd++]   = wpm_str[j];
    for (int j = 0; j < time_len;  j++) time_display[td++]  = time_str[j];
    for (int j = 0; j < bonus_len; j++) bonus_display[bd++] = bonus_str[j];

    acc_display[ad]   = '\0'; wpm_display[wd]   = '\0';
    time_display[td]  = '\0'; bonus_display[bd] = '\0';

    int max_len = ad;
    if (wd > max_len) max_len = wd;
    if (td > max_len) max_len = td;
    if (bd > max_len) max_len = bd;
    if (score_len > max_len) max_len = score_len;

    int block_x  = center_x - (max_len * chw) / 2;
    int name_len = strlen(p->name);

    color_t bg_colors[] = {
        0xFF1A0000, 0xFF1A000F, 0xFF0F001A,
        0xFF00001A, 0xFF001A0F, 0xFF001A00,
    };
    color_t fg_colors[] = {
        GL_RED, GL_MAGENTA, 0xFFAA00FF,
        GL_CYAN, GL_GREEN, 0xFF00FF88,
    };

    int winner_x  = center_x - (7  * chw) / 2;
    int winner_y  = f->y + f->height - 6 * chh;
    int restart_x = center_x - (20 * chw) / 2;
    int restart_y = winner_y + 2 * chh;

    unsigned int last_tick = 0;
    int color_idx = 0;
    while (true) {
        char c;
        if (keyboard_try_read_next(kb1, &c) && c == '\t') return;
        if (keyboard_try_read_next(kb2, &c) && c == '\t') return;
        unsigned int tick = timer_get_ticks() / (TICKS_PER_USEC * 300000);
        if (tick == last_tick) continue;
        last_tick  = tick;
        color_idx  = tick % 6;
        color_t bg = bg_colors[color_idx];
        color_t fg = fg_colors[color_idx];
        for (int ii = 0; ii < 2; ii++) {
            gl_draw_rect(f->x, f->y, f->width, f->height, bg);
            gl_draw_string(center_x - (name_len * chw) / 2, base_y,           p->name,       GL_WHITE);
            gl_draw_string(block_x,  base_y + line_h + 8,                      acc_display,   GL_WHITE);
            gl_draw_string(block_x,  base_y + line_h * 2 + 8,                  wpm_display,   GL_WHITE);
            gl_draw_string(block_x,  base_y + line_h * 3 + 8,                  time_display,  GL_WHITE);
            gl_draw_string(block_x,  base_y + line_h * 4 + 8,                  bonus_display, GL_WHITE);
            gl_draw_rect(block_x,    base_y + line_h * 5 + 8, max_len * chw, 2, GL_WHITE);
            gl_draw_string(block_x,  base_y + line_h * 5 + 20,                 score_str,     fg);
            gl_draw_string(winner_x, winner_y,                                  "WINNER!",     fg);
            gl_draw_string(restart_x, restart_y,                                "Press TAB to restart", GL_SILVER);
            gl_swap_buffer();
        }
    }
}

void render_finish_overlay(game_state_t *game, int player_num) {
    frame *f = (player_num == 1) ? &display.f1 : &display.f2;
    color_t dark = 0xFF0A0A0A;

    for (int i = 0; i < 2; i++) {
        gl_draw_rect(f->x, f->y, f->width, f->height, f->bgcolor);
        gl_swap_buffer();
    }
    for (int s = 1; s <= 20; s++) {
        int h = (f->height * s * s) / 400;
        gl_draw_rect(f->x, f->y, f->width, h, dark); gl_swap_buffer();
        gl_draw_rect(f->x, f->y, f->width, h, dark); gl_swap_buffer();
    }
    for (int i = 0; i < 2; i++) {
        gl_draw_rect(f->x, f->y, f->width, f->height, dark);
        gl_swap_buffer();
    }
}

void render_finish_stats(game_state_t *game, int player_num) {
    frame    *f = (player_num == 1) ? &display.f1 : &display.f2;
    player_t *p = (player_num == 1) ? &game->p1   : &game->p2;
    color_t dark = 0xFF0A0A0A;
    int chh      = gl_get_char_height();
    int chw      = gl_get_char_width();
    int center_x = f->x + f->width / 2;
    int line_h   = chh + LINE_SPACING * 2;
    int base_y   = f->y + f->height / 2 - 5 * line_h;
    int start_y  = base_y;

    int acc = player_get_accuracy(p);
    int wpm = player_get_wpm(p);
    int remaining_secs = 0;
    if (p->start_time != 0 && game->duration_secs > 0) {
        unsigned int elapsed = (timer_get_ticks() - p->start_time) / (TICKS_PER_USEC * 1000000);
        remaining_secs = game->duration_secs - (int)elapsed;
        if (remaining_secs < 0) remaining_secs = 0;
    }
    int time_bonus = remaining_secs * 10;
    int bonus_pts  = p->bonus_score * 100;
    int score      = wpm * acc + bonus_pts + time_bonus;

    p->final_wpm        = wpm;
    p->final_acc        = acc;
    p->final_time_bonus = time_bonus;
    p->final_score      = score;

    char buf[12];
    char acc_str[32];   int acc_len   = 0;
    char wpm_str[32];   int wpm_len   = 0;
    char time_str[32];  int time_len  = 0;
    char bonus_str[32]; int bonus_len = 0;
    char score_str[32]; int score_len = 0;

    const char *acc_label   = "Accuracy: ";
    const char *wpm_label   = "WPM:      ";
    const char *time_label  = "Time:    +";
    const char *bonus_label = "Bonus:   +";
    const char *score_label = "Score:      ";

    for (int j = 0; acc_label[j];   j++) acc_str[acc_len++]     = acc_label[j];
    int_to_str_padded(acc, buf, 6);
    for (int j = 0; buf[j]; j++) acc_str[acc_len++] = buf[j];
    acc_str[acc_len++] = '%'; acc_str[acc_len] = '\0';

    for (int j = 0; wpm_label[j];   j++) wpm_str[wpm_len++]     = wpm_label[j];
    int_to_str_padded(wpm, buf, 6);
    for (int j = 0; buf[j]; j++) wpm_str[wpm_len++] = buf[j];
    wpm_str[wpm_len] = '\0';

    for (int j = 0; time_label[j];  j++) time_str[time_len++]   = time_label[j];
    int_to_str_padded(time_bonus, buf, 6);
    for (int j = 0; buf[j]; j++) time_str[time_len++] = buf[j];
    time_str[time_len] = '\0';

    for (int j = 0; bonus_label[j]; j++) bonus_str[bonus_len++] = bonus_label[j];
    int_to_str_padded(bonus_pts, buf, 6);
    for (int j = 0; buf[j]; j++) bonus_str[bonus_len++] = buf[j];
    bonus_str[bonus_len] = '\0';

    for (int j = 0; score_label[j]; j++) score_str[score_len++] = score_label[j];
    int_to_str_padded(score, buf, 6);
    for (int j = 0; buf[j]; j++) score_str[score_len++] = buf[j];
    score_str[score_len] = '\0';

    char acc_display[40];   int ad = 0;
    char wpm_display[40];   int wd = 0;
    char time_display[40];  int td = 0;
    char bonus_display[40]; int bd = 0;

    acc_display[ad++]   = 'x'; acc_display[ad++]   = ' ';
    wpm_display[wd++]   = 'x'; wpm_display[wd++]   = ' ';
    time_display[td++]  = '+'; time_display[td++]  = ' ';
    bonus_display[bd++] = '+'; bonus_display[bd++] = ' ';

    for (int j = 0; j < acc_len;   j++) acc_display[ad++]   = acc_str[j];
    for (int j = 0; j < wpm_len;   j++) wpm_display[wd++]   = wpm_str[j];
    for (int j = 0; j < time_len;  j++) time_display[td++]  = time_str[j];
    for (int j = 0; j < bonus_len; j++) bonus_display[bd++] = bonus_str[j];

    acc_display[ad]   = '\0'; wpm_display[wd]   = '\0';
    time_display[td]  = '\0'; bonus_display[bd] = '\0';

    int max_len = ad;
    if (wd > max_len) max_len = wd;
    if (td > max_len) max_len = td;
    if (bd > max_len) max_len = bd;
    if (score_len > max_len) max_len = score_len;

    int block_x  = center_x - (max_len * chw) / 2;
    int name_len = strlen(p->name);

#define DRAW_BOTH(expr) do { expr; gl_swap_buffer(); expr; gl_swap_buffer(); } while(0)
#define PAUSE()         do { for (volatile int d = 0; d < 2000000; d++) {} } while(0)

    DRAW_BOTH(gl_draw_string(center_x - (name_len * chw) / 2, start_y, p->name, GL_WHITE));
    start_y += line_h + 8; PAUSE();
    DRAW_BOTH(gl_draw_string(block_x, start_y, acc_display,   GL_AMBER));
    start_y += line_h;     PAUSE();
    DRAW_BOTH(gl_draw_string(block_x, start_y, wpm_display,   GL_AMBER));
    start_y += line_h;     PAUSE();
    DRAW_BOTH(gl_draw_string(block_x, start_y, time_display,  GL_GREEN));
    start_y += line_h;     PAUSE();
    DRAW_BOTH(gl_draw_string(block_x, start_y, bonus_display, GL_GREEN));
    start_y += line_h;     PAUSE();
    DRAW_BOTH(gl_draw_rect(block_x, start_y, max_len * chw, 2, GL_WHITE));
    start_y += 12;         PAUSE();
    DRAW_BOTH(gl_draw_string(block_x, start_y, score_str, GL_GREEN));

    for (int ii = 0; ii < 2; ii++) {
        gl_draw_rect(f->x, f->y, f->width, f->height, dark);
        gl_draw_string(center_x - (name_len * chw) / 2, base_y,           p->name,       GL_WHITE);
        gl_draw_string(block_x,  base_y + line_h + 8,                      acc_display,   GL_AMBER);
        gl_draw_string(block_x,  base_y + line_h * 2 + 8,                  wpm_display,   GL_AMBER);
        gl_draw_string(block_x,  base_y + line_h * 3 + 8,                  time_display,  GL_GREEN);
        gl_draw_string(block_x,  base_y + line_h * 4 + 8,                  bonus_display, GL_GREEN);
        gl_draw_rect(block_x,    base_y + line_h * 5 + 8, max_len * chw, 2, GL_WHITE);
        gl_draw_string(block_x,  base_y + line_h * 5 + 20,                 score_str,     GL_GREEN);
        gl_swap_buffer();
    }

#undef DRAW_BOTH
#undef PAUSE
}

void render_word(textbox *tb, player_t *player, int target_word_idx, color_t bgcolor, game_state_t *game) {
    int player_num = (player == &game->p1) ? 1 : 2;
    int n   = player->promptlen;
    int chw = gl_get_char_width();
    int chh = gl_get_char_height() + LINE_SPACING;
    int row = 0, col = 0, i = 0, word_idx = 0;

    while (i < n) {
        int wlen = 0;
        while (i + wlen < n && player->promptstr[i + wlen] != ' ') wlen++;
        if (col + wlen + 1 > tb->ncols) { col = 0; row++; }
        if (row >= tb->nrows) break;

        if (word_idx == target_word_idx) {
            for (int j = 0; j < wlen; j++) {
                color_t c = get_char_color(game, player, word_idx, i, player_num, bgcolor);
                gl_draw_rect(tb->x + col * chw, tb->y + row * chh, chw, chh, bgcolor);
                gl_draw_char(tb->x + col * chw, tb->y + row * chh, player->promptstr[i], c);
                col++; i++;
            }
            return;
        }
        col += wlen; i += wlen;
        if (i < n && player->promptstr[i] == ' ') {
            col++; i++;
            if (col >= tb->ncols) { col = 0; row++; }
        }
        word_idx++;
    }
}

void render(game_state_t *game) {
    frame *f1 = &display.f1;
    frame *f2 = &display.f2;

    int changed1 = game->p1.last_was_backspace ? game->p1.cursor : game->p1.cursor - 1;
    int changed2 = game->p2.last_was_backspace ? game->p2.cursor : game->p2.cursor - 1;
    int ox1 = game->p1.old_cursor_x, oy1 = game->p1.old_cursor_y;
    int ox2 = game->p2.old_cursor_x, oy2 = game->p2.old_cursor_y;

    bool p1_did_redraw = false;
    bool p2_did_redraw = false;

    for (int i = 0; i < 2; i++) {
        if (game->p1.needs_full_redraw && !game->p1_screen_done) {
            gl_draw_rect(f1->x, f1->tb.y, f1->width, f1->tb.height, f1->bgcolor);
            render_text_rows(&f1->tb, &game->p1, 0, 999, f1->bgcolor, game);
            p1_did_redraw = true;
        }
        if (game->p2.needs_full_redraw && !game->p2_screen_done) {
            gl_draw_rect(f2->x, f2->tb.y, f2->width, f2->tb.height, f2->bgcolor);
            render_text_rows(&f2->tb, &game->p2, 0, 999, f2->bgcolor, game);
            p2_did_redraw = true;
        }

        if (!game->p1_screen_done) {
            gl_draw_rect(ox1, oy1, 2, gl_get_char_height(), f1->bgcolor);
            redraw_char_at_pixel(&f1->tb, &game->p1, ox1, oy1, f1->bgcolor, game);
            if (game->p1.cursor > 0)
                render_char_at(&f1->tb, &game->p1, game->p1.cursor - 1, f1->bgcolor, game);
            if (game->p1.cursor < game->p1.promptlen)
                render_char_at(&f1->tb, &game->p1, game->p1.cursor,     f1->bgcolor, game);
            if (game->p1.cursor + 1 < game->p1.promptlen)
                render_char_at(&f1->tb, &game->p1, game->p1.cursor + 1, f1->bgcolor, game);
            if (changed1 >= 0)
                render_char_at(&f1->tb, &game->p1, changed1, f1->bgcolor, game);
            if (game->p1.cursor < game->p1.promptlen)
                gl_draw_rect(game->p1.cursor_x, game->p1.cursor_y, 2, gl_get_char_height(), COLOR_CURSOR);
            if (game->p1_message)
                render_message(1, game->p1_message);
        }

        if (!game->p2_screen_done) {
            gl_draw_rect(ox2, oy2, 2, gl_get_char_height(), f2->bgcolor);
            redraw_char_at_pixel(&f2->tb, &game->p2, ox2, oy2, f2->bgcolor, game);
            if (game->p2.cursor > 0)
                render_char_at(&f2->tb, &game->p2, game->p2.cursor - 1, f2->bgcolor, game);
            if (game->p2.cursor < game->p2.promptlen)
                render_char_at(&f2->tb, &game->p2, game->p2.cursor,     f2->bgcolor, game);
            if (game->p2.cursor + 1 < game->p2.promptlen)
                render_char_at(&f2->tb, &game->p2, game->p2.cursor + 1, f2->bgcolor, game);
            if (changed2 >= 0)
                render_char_at(&f2->tb, &game->p2, changed2, f2->bgcolor, game);
            if (game->p2.cursor < game->p2.promptlen)
                gl_draw_rect(game->p2.cursor_x, game->p2.cursor_y, 2, gl_get_char_height(), COLOR_CURSOR);
            if (game->p2_message)
                render_message(2, game->p2_message);
        }

        if (game->bonus_just_claimed) {
            if (!game->p1_screen_done)
                render_word(&f1->tb, &game->p1, game->bonus_just_claimed_idx, f1->bgcolor, game);
            if (!game->p2_screen_done)
                render_word(&f2->tb, &game->p2, game->bonus_just_claimed_idx, f2->bgcolor, game);
            if (i == 1) game->bonus_just_claimed = false;
        }

        if (!game->p1_message && !game->p1_screen_done) {
            int chh = gl_get_char_height(), pad = 20;
            int bh  = chh + 2 * pad;
            int by  = f1->y + RES_HEIGHT - bh - pad;
            gl_draw_rect(f1->x + pad, by, f1->width - 2 * pad, bh, f1->bgcolor);
        }
        if (!game->p2_message && !game->p2_screen_done) {
            int chh = gl_get_char_height(), pad = 20;
            int bh  = chh + 2 * pad;
            int by  = f2->y + RES_HEIGHT - bh - pad;
            gl_draw_rect(f2->x + pad, by, f2->width - 2 * pad, bh, f2->bgcolor);
        }

        render_stats(game);
        gl_swap_buffer();
    }

    if (p1_did_redraw) game->p1.needs_full_redraw = false;
    if (p2_did_redraw) game->p2.needs_full_redraw = false;
}

void render_stats_only(game_state_t *game) {
    for (int i = 0; i < 2; i++) {
        render_stats(game);
        gl_swap_buffer();
    }
}

typedef struct {
    const char *desc;
    int descx, descy;
    int x, y, width, height;
} field;

static field get_field(frame *f, int width, const char *desc) {
    int height = gl_get_char_height() + 2 * LINE_SPACING;
    int line   = gl_get_char_height() + LINE_SPACING;
    int descx  = (f->width - width) / 2 + f->x;
    int descy  = (f->height - height - line) / 2 + f->y;
    return (field){ desc, descx, descy, descx, descy + line, width, height };
}

static field get_select(int y, int width, const char *desc) {
    int height = gl_get_char_height() + 2 * LINE_SPACING;
    int line   = gl_get_char_height() + LINE_SPACING;
    int descx  = (RES_WIDTH - width) / 2;
    return (field){ desc, descx, y, descx, y + line, width, height };
}

static void draw_field(const char *content, field f, color_t color, bool active) {
    gl_draw_string(f.descx, f.descy, f.desc, active ? COLOR_SELECT : GL_WHITE);
    gl_draw_rect(f.x, f.y, f.width, f.height, active ? COLOR_SELECT : GL_WHITE);
    gl_draw_rect(f.x + TB_BORDER, f.y + TB_BORDER,
                 f.width - 2 * TB_BORDER, f.height - 2 * TB_BORDER, color + 0x101010);
    gl_draw_string(f.x + TB_PAD, f.y + LINE_SPACING, content, GL_WHITE);
}

void render_menu(game_state_t *game, select active) {
    frame *f1 = &display.f1;
    frame *f2 = &display.f2;
    gl_draw_rect(f1->x, f1->y, f1->width, f1->height, f1->bgcolor);
    gl_draw_rect(f2->x, f2->y, f2->width, f2->height, f2->bgcolor);

    settings *set = &game->set;
    draw_field(set->p1, get_field(f1, f1->width / 3, "Player 1 Name"), COLOR_P1_BG, active == P1_NAME);
    draw_field(set->p2, get_field(f2, f2->width / 3, "Player 2 Name"), COLOR_P2_BG, active == P2_NAME);

    draw_field(set->timer,    get_select(RES_HEIGHT / 2 - 150, RES_WIDTH / 15, "Timer (s)"),  COLOR_N_BG, active == TIME);
    draw_field(set->powerups, get_select(RES_HEIGHT / 2 - 50,  RES_WIDTH / 15, "Powerups"),   COLOR_N_BG, active == POWERUPS);
    draw_field(set->prompt,   get_select(RES_HEIGHT / 2 + 50,  RES_WIDTH / 15, "# Words"),    COLOR_N_BG, active == PROMPT);

    if (set->error_msg) {
        int chw = gl_get_char_width();
        int msg_x = (RES_WIDTH - (int)strlen(set->error_msg) * chw) / 2;
        gl_draw_string(msg_x, RES_HEIGHT / 2 + 150, set->error_msg, GL_RED);
    }
    gl_swap_buffer();
}