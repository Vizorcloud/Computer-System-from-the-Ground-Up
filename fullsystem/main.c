/* File: main.c
 * ------------
 * Console program running on student's library libmymango
 * Used for integration test of fullsystem.
 */
#include "console.h"
#include "interrupts.h"
#include "keyboard.h"
#include "shell.h"
#include "timer.h"
#include "uart.h"

void main(void) {
    interrupts_init();
    gpio_init();
    timer_init();
    uart_init();

    console_startup_screen();
    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
    console_init(30, 80, GL_GREEN, GL_BLACK);
    shell_init(keyboard_read_next, console_printf);

    interrupts_global_enable();
    shell_run();
}
