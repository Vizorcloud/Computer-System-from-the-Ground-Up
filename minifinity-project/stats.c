#include "stats.h"
#include "timer.h"
#include "strings.h"

int count_correct_word_chars(player_t* p) {
    int i = 0, correct = 0;
    while (i < p->promptlen) {
        int word_start = i;
        while (i < p->promptlen && p->promptstr[i] != ' ') i++;
        int word_len = i - word_start;
        bool word_correct = true;
        for (int j = word_start; j < word_start + word_len; j++) {
        if (p->acc[j] != RIGHT) { word_correct = false; break; }
                }
        if (word_correct) correct += word_len + 1;
        if (i < p->promptlen) i++;
    }
    return correct;
}

int player_get_wpm(player_t* p) {
    if (p->start_time == 0) return 0;
    unsigned int elapsed = (timer_get_ticks() - p->start_time) / (TICKS_PER_USEC * 1000000);
    if (elapsed == 0) return 0;
    return (count_correct_word_chars(p) * 60) / (5 * elapsed);
}

int player_get_raw_wpm(player_t* p) {
    if (p->start_time == 0) return 0;
    unsigned int elapsed = (timer_get_ticks() - p->start_time) / (TICKS_PER_USEC * 1000000);
    if (elapsed == 0) return 0;
    return (p->raw_chars * 60) / (5 * elapsed);
}

int player_get_accuracy(player_t* p) {
    if (p->raw_chars == 0) return 100;
    if (p->error_chars > p->raw_chars) return 0;
    return ((p->raw_chars - p->error_chars) * 100) / p->raw_chars;
}