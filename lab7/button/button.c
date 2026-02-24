#include "gl.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupt.h"
#include "interrupts.h"
#include "printf.h"
#include "ringbuffer.h"
#include "uart.h"

static const gpio_id_t BUTTON = GPIO_PB4;
static int gCount = 0;

void wait_for_click(void) {
    // TODO: implement this function to
    // wait for falling edge on button
    // after see falling edge, increment count & output message
}

void redraw(int nclicks) {
    static int nredraw = -1;
    // count number of redraws, alternate bg color on each redraw
    color_t bg = nredraw++ % 2 ? GL_AMBER : GL_BLUE;

    gl_clear(GL_BLACK);
    char buf[100];
    snprintf(buf, sizeof(buf), "Click count = %d (redraw #%d)", nclicks, nredraw);
    gl_draw_string(0, 0, buf, GL_WHITE);
    // repaint rest of screen using an intentionally slow loop for educational purposes :-)
    for (int y = gl_get_char_height(); y < gl_get_height(); y++) {
        for (int x = 0; x < gl_get_width(); x++) {
            gl_draw_pixel(x, y, bg);
        }
    }
}

void config_button(void) {
    gpio_set_input(BUTTON);
    gpio_set_pullup(BUTTON); // use internal pullup resistor
    // add code here to config gpio interrupt for button
}

void main(void) {
    gpio_init();
    uart_init();
    gl_init(1000, 700, GL_SINGLEBUFFER);
    config_button();

    int drawn = 0;
    redraw(drawn);
    while (true) {
        wait_for_click();
        drawn = gCount;
        redraw(drawn);
    }
}
