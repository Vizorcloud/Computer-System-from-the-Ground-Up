#include "uart.h"
#include "render.h"
#include "game.h"
#include "keyboard.h"
#include "interrupts.h"
#include "timer.h"
#include "printf.h"
#include "words.h"

void main(void) {
    interrupts_init();
    gpio_init();
    timer_init();
    uart_init();

    game_init(render_field_size());

    keyboard_t* kb1 = keyboard_init(KEYBOARD_CLOCK1, KEYBOARD_DATA1);
    keyboard_t* kb2 = keyboard_init(KEYBOARD_CLOCK2, KEYBOARD_DATA2);

    interrupts_global_enable();

    game_run(kb1, kb2);
}
