#ifndef STATS_H
#define STATS_H
#include "game.h"

int count_correct_word_chars(player_t* p);
int player_get_wpm(player_t* p);
int player_get_raw_wpm(player_t* p);
int player_get_accuracy(player_t* p);

#endif