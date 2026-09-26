#include "game.h"
#include "keyboard.h"
#include "malloc.h"
#include "render.h"
#include "strings.h"
#include "prompt.h"
#include "timer.h"
#include "rand.h"
#include "stats.h"

static game_state_t state;
static int MAXFIELD;
#define FRAME_INTERVAL_US 16667

static unsigned int last_tick  = 0;
static bool         input_dirty = false;

static bool should_render(void) {
    unsigned int now = timer_get_ticks();
    if (now - last_tick >= FRAME_INTERVAL_US) {
        last_tick = now;
        return true;
    }
    return false;
}

void game_init(int maxfield) {
    MAXFIELD = maxfield;

    state.p1.cursor = 0;
    state.p2.cursor = 0;

    state.p1.cursor_x = 0;     state.p1.cursor_y = 0;
    state.p1.old_cursor_x = 0; state.p1.old_cursor_y = 0;
    state.p1.target_x = 0;     state.p1.target_y = 0;
    state.p1.last_was_backspace = false;

    state.p2.cursor_x = 0;     state.p2.cursor_y = 0;
    state.p2.old_cursor_x = 0; state.p2.old_cursor_y = 0;
    state.p2.target_x = 0;     state.p2.target_y = 0;
    state.p2.last_was_backspace = false;

    state.p1.needs_full_redraw = false;
    state.p2.needs_full_redraw = false;

    state.p1.raw_chars = 0;  state.p1.error_chars = 0;  state.p1.start_time = 0;
    state.p2.raw_chars = 0;  state.p2.error_chars = 0;  state.p2.start_time = 0;

    state.p1.active_powerup = POWERUP_NONE;  state.p1.powerup_end_time = 0;
    state.p2.active_powerup = POWERUP_NONE;  state.p2.powerup_end_time = 0;
    state.p1.flipped = false;
    state.p2.flipped = false;

    state.p1.promptlen = 0;  state.p1.promptstr = NULL;  state.p1.acc = NULL;
    state.p2.promptlen = 0;  state.p2.promptstr = NULL;  state.p2.acc = NULL;

    state.set.p1[0] = '\0';
    state.set.p2[0] = '\0';
    memcpy(state.set.timer,    "60", MAX_SET_LEN);  state.set.timer[MAX_SET_LEN - 1]    = '\0';
    memcpy(state.set.prompt,   "50", MAX_SET_LEN);  state.set.prompt[MAX_SET_LEN - 1]   = '\0';
    memcpy(state.set.powerups, "3",  MAX_SET_LEN);  state.set.powerups[MAX_SET_LEN - 1] = '\0';

    state.start_time        = 0;
    state.p1_message        = NULL;
    state.p2_message        = NULL;
    state.num_bonus_words   = 3;
    state.p1.bonus_score    = 0;
    state.p2.bonus_score    = 0;
    state.p1_finished       = false;
    state.p2_finished       = false;
    state.bonus_just_claimed = false;
    state.p1_screen_done    = false;
    state.p2_screen_done    = false;
    state.set.error_msg     = NULL;
    state.set.ready         = false;
}

void game_update(player_t *p, char ch, int player_num) {
    if (ch == '\b') {
        if (p->cursor > 0) {
            p->cursor--;
            p->acc[p->cursor] = UNTYPED;
            p->last_was_backspace = true;
        }
        return;
    }
    p->last_was_backspace = false;
    if (p->cursor >= p->promptlen) return;

    if (p->promptstr[p->cursor] == ' ' && ch != ' ') return;
    if (p->start_time == 0) p->start_time = timer_get_ticks();

    p->raw_chars++;
    if (ch == p->promptstr[p->cursor]) {
        p->acc[p->cursor] = RIGHT;
    } else {
        p->acc[p->cursor] = WRONG;
        p->error_chars++;
    }
    p->cursor++;

    if (p->promptlen > 0 && p->cursor == p->promptlen && p->acc[p->promptlen - 1] == RIGHT) {
        if (player_num == 1) state.p1_finished = true;
        else                 state.p2_finished = true;
    }

    if (p->promptlen > 0 && (p->cursor == p->promptlen || p->promptstr[p->cursor] == ' ')) {
        int word_end   = p->cursor;
        int word_start = word_end - 1;
        while (word_start > 0 && p->promptstr[word_start - 1] != ' ') word_start--;

        int word_idx = 0;
        for (int i = 0; i < word_start; i++)
            if (p->promptstr[i] == ' ') word_idx++;

        bool all_right = true;
        for (int j = word_start; j < word_end; j++) {
            if (p->acc[j] != RIGHT) { all_right = false; break; }
        }

        if (all_right) {
            for (int b = 0; b < state.num_bonus_words; b++) {
                if (state.bonus_word_idxs[b] != word_idx || state.bonus_word_claimed[b]) continue;

                state.bonus_word_claimed[b]   = true;
                state.bonus_word_claimer[b]   = player_num;
                state.bonus_just_claimed      = true;
                state.bonus_just_claimed_idx  = word_idx;
                p->bonus_score++;

                player_t *opponent = (player_num == 1) ? &state.p2 : &state.p1;
                powerup_type pt    = state.bonus_word_powerup[b];
                opponent->active_powerup   = pt;
                opponent->powerup_end_time = timer_get_ticks() + (unsigned int)(3 * TICKS_PER_USEC * 1000000);
                opponent->flipped          = (pt == POWERUP_FLIP);

                if (player_num == 1) {
                    state.p1_message = "Powerup used on P2!";
                    if (pt == POWERUP_FLIP)    state.p2_message = "Incoming: text flipped!";
                    if (pt == POWERUP_CAMO)    state.p2_message = "Incoming: letters hidden!";
                    if (pt == POWERUP_RAINBOW) state.p2_message = "Incoming: rainbow text!";
                } else {
                    state.p2_message = "Powerup used on P1!";
                    if (pt == POWERUP_FLIP)    state.p1_message = "Incoming: text flipped!";
                    if (pt == POWERUP_CAMO)    state.p1_message = "Incoming: letters hidden!";
                    if (pt == POWERUP_RAINBOW) state.p1_message = "Incoming: rainbow text!";
                }
                opponent->needs_full_redraw = true;
            }
        }
    }
}

static void kb_input(keyboard_t *kb, player_t *p, int player_num) {
    char c;
    if (keyboard_try_read_next(kb, &c)) {
        game_update(p, c, player_num);
        input_dirty = true;
    }
}

static void update_field(char *field, int maxlen, char c) {
    int len = strlen(field);
    if (c == '\b') {
        if (len > 0) field[len - 1] = '\0';
    } else if (len < maxlen - 1) {
        field[len]     = c;
        field[len + 1] = '\0';
    }
}

static bool isnum(char c) {
    return c >= '0' && c <= '9';
}

static void kb_select(keyboard_t *kb, select *s) {
    char c;
    if (!keyboard_try_read_next(kb, &c)) return;

    if (c == '\t') {
        *s = (*s + 1) % N_SELECT;
    } else if (c == '\n') {
        int nwords    = strtonum(state.set.prompt,   NULL);
        int npowerups = strtonum(state.set.powerups, NULL);
        if (strlen(state.set.p1) == 0 || strlen(state.set.p2) == 0)
            state.set.error_msg = "Both players need a name!";
        else if (nwords < npowerups)
            state.set.error_msg = "Words must be >= powerups!";
        else {
            state.set.error_msg = NULL;
            state.set.ready     = true;
        }
        render_menu(&state, *s);
    } else {
        if (*s == P1_NAME)                            update_field(state.set.p1,       MAX_NAME_LEN, c);
        if (*s == P2_NAME)                            update_field(state.set.p2,       MAX_NAME_LEN, c);
        if (*s == PROMPT   && (isnum(c) || c == '\b')) update_field(state.set.prompt,   MAX_SET_LEN,  c);
        if (*s == TIME     && (isnum(c) || c == '\b')) update_field(state.set.timer,    MAX_SET_LEN,  c);
        if (*s == POWERUPS && (isnum(c) || c == '\b')) update_field(state.set.powerups, MAX_SET_LEN,  c);
    }
    render_menu(&state, *s);
}

void game_run(keyboard_t *kb1, keyboard_t *kb2) {
    select s1 = P1_NAME;
    select s2 = P1_NAME;

restart:
    game_init(0);
    s1 = P1_NAME;
    s2 = P1_NAME;

    { char c; while (keyboard_try_read_next(kb1, &c)) {} }
    { char c; while (keyboard_try_read_next(kb2, &c)) {} }

    render_init(50);
    render_menu(&state, s1);
    while (!state.set.ready) {
        kb_select(kb1, &s1);
        kb_select(kb2, &s2);
    }

    state.nwords       = strtonum(state.set.prompt, NULL);
    if (state.nwords <= 0) state.nwords = 50;
    state.p1.name      = state.set.p1;
    state.p2.name      = state.set.p2;

    state.duration_secs = strtonum(state.set.timer, NULL);
    if (state.duration_secs <= 0 || state.duration_secs > 3600) state.duration_secs = 60;

    state.num_bonus_words = strtonum(state.set.powerups, NULL);
    if (state.num_bonus_words < 0)                state.num_bonus_words = 0;
    if (state.num_bonus_words > MAX_BONUS_WORDS)  state.num_bonus_words = MAX_BONUS_WORDS;

    render_countdown();
    render_init(state.nwords);
    srand(timer_get_ticks());

    state.p1.prompt = getprompt(state.nwords);
    state.p2.prompt = getprompt(state.nwords);

    powerup_type available[] = {POWERUP_FLIP, POWERUP_CAMO, POWERUP_RAINBOW, POWERUP_FLIP, POWERUP_CAMO};
    for (int i = 4; i > 0; i--) {
        int j = rand() % (i + 1);
        powerup_type tmp = available[i];
        available[i]     = available[j];
        available[j]     = tmp;
    }
    for (int i = 0; i < state.num_bonus_words; i++) {
        int idx;
        bool unique;
        do {
            unique = true;
            idx = rand() % state.nwords;
            for (int j = 0; j < i; j++) {
                if (state.bonus_word_idxs[j] == idx) { unique = false; break; }
            }
        } while (!unique);
        state.bonus_word_idxs[i]    = idx;
        state.bonus_word_powerup[i] = available[i % 4];
    }

    render_start(&state);
    state.start_time = timer_get_ticks();
    last_tick        = state.start_time;

    while (true) {
        kb_input(kb1, &state.p1, 1);
        kb_input(kb2, &state.p2, 2);

        unsigned int now = timer_get_ticks();
        if (state.p1.active_powerup != POWERUP_NONE && now >= state.p1.powerup_end_time) {
            state.p1.active_powerup    = POWERUP_NONE;
            state.p1.flipped           = false;
            state.p1.needs_full_redraw = true;
            state.p1_message           = NULL;
            state.p2_message           = NULL;
        }
        if (state.p2.active_powerup != POWERUP_NONE && now >= state.p2.powerup_end_time) {
            state.p2.active_powerup    = POWERUP_NONE;
            state.p2.flipped           = false;
            state.p2.needs_full_redraw = true;
            state.p1_message           = NULL;
            state.p2_message           = NULL;
        }

        if (state.p1_finished) {
            render_finish_overlay(&state, 1);
            state.p1_finished    = false;
            state.p1_screen_done = true;
        }
        if (state.p2_finished) {
            render_finish_overlay(&state, 2);
            state.p2_finished    = false;
            state.p2_screen_done = true;
        }

        if (state.p1_screen_done && state.p2_screen_done) {
            render_finish_stats(&state, 1);
            render_finish_stats(&state, 2);
            int winner = (state.p1.final_score >= state.p2.final_score) ? 1 : 2;
            render_celebrate(&state, winner, kb1, kb2);
            goto restart;
        }

        if (state.start_time != 0) {
            unsigned int elapsed = (timer_get_ticks() - state.start_time) / (TICKS_PER_USEC * 1000000);
            if (elapsed >= (unsigned int)state.duration_secs) {
                if (!state.p1_screen_done) render_finish_overlay(&state, 1);
                if (!state.p2_screen_done) render_finish_overlay(&state, 2);
                render_finish_stats(&state, 1);
                render_finish_stats(&state, 2);
                int winner = (state.p1.final_score >= state.p2.final_score) ? 1 : 2;
                render_celebrate(&state, winner, kb1, kb2);
                goto restart;
            }
        }

        if (input_dirty) {
            state.p1.old_cursor_x = state.p1.cursor_x;
            state.p1.old_cursor_y = state.p1.cursor_y;
            state.p2.old_cursor_x = state.p2.cursor_x;
            state.p2.old_cursor_y = state.p2.cursor_y;
            render_step_cursors(&state);
            render(&state);
            input_dirty = false;
            last_tick   = timer_get_ticks();
        } else if (should_render()) {
            state.p1.old_cursor_x = state.p1.cursor_x;
            state.p1.old_cursor_y = state.p1.cursor_y;
            state.p2.old_cursor_x = state.p2.cursor_x;
            state.p2.old_cursor_y = state.p2.cursor_y;
            render_step_cursors(&state);
            render(&state);
        }
    }
}