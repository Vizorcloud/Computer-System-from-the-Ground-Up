#include "timer.h"

void timer_init() {}

void timer_delay(int seconds) {
    timer_delay_ms(1000*seconds); // wait, where is timer_delay_ms function?
}