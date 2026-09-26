#ifndef GAME_H
#define GAME_H
#include "keyboard.h"
#define MAX_WORDS 100
#define MAX_BONUS_WORDS 5
#define MAX_NAME_LEN 17
#define MAX_SET_LEN 4

typedef enum {
    UNTYPED,
    WRONG,
    RIGHT,
} result;

typedef enum {
    P1_NAME, P2_NAME, TIME, POWERUPS, PROMPT,
    N_SELECT
} select;

typedef enum {
    POWERUP_NONE = 0,
    POWERUP_FLIP,
    POWERUP_CAMO,
    POWERUP_RAINBOW,
} powerup_type;

typedef struct {
    const char** prompt;
    int cursor;
    char* promptstr;
    int promptlen;
    char* name;
    result* acc;
    int cursor_x;
    int cursor_y;
    int target_x;
    int target_y;
    bool last_was_backspace;
    unsigned int start_time;
    int raw_chars;
    int error_chars;
    bool flipped;
    int old_cursor_x;
    int old_cursor_y;
    int bonus_score;
    powerup_type active_powerup;
    unsigned int powerup_end_time;
    bool needs_full_redraw;
    // In player_t (game.h), add:
    int final_wpm;
    int final_acc;
    int final_time_bonus;
    int final_score;
} player_t;

typedef struct {
    char p1[MAX_NAME_LEN];
    char p2[MAX_NAME_LEN];
    char timer[MAX_SET_LEN];
    char prompt[MAX_SET_LEN];
    char powerups[MAX_SET_LEN];
    bool ready, timerok, promptok;
    const char* error_msg;
} settings;

typedef struct {
    player_t p1;
    player_t p2;
    int nwords;
    unsigned int start_time;
    int duration_secs;
    int bonus_word_idxs[MAX_BONUS_WORDS];
    int num_bonus_words;
    bool bonus_word_claimed[MAX_BONUS_WORDS];
    powerup_type bonus_word_powerup[MAX_BONUS_WORDS];
    const char* p1_message;
    const char* p2_message;
    int bonus_word_claimer[MAX_BONUS_WORDS];
    bool bonus_just_claimed;
    int bonus_just_claimed_idx;
    settings set;
    bool p1_finished;
    bool p2_finished;
    bool p1_screen_done;
    bool p2_screen_done;
} game_state_t;

void game_init(int maxfield);
void game_run(keyboard_t* kb1, keyboard_t* kb2);
void game_update(player_t* p, char ch, int player_num);
#endif