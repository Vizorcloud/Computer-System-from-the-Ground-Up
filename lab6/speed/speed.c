/* File speed.c
 * ------------
 * Simple main program with timing code to measure
 * performance of different implementations of redraw.
 */

#include "fb.h"
#include "gl.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"

// -------------------------------------------------------------------
// Time Trial Helpers
// -------------------------------------------------------------------

static void _time_trial(const char *f_name, void (*fn)(color_t), const char *c_name, color_t color) {
    unsigned long start = timer_get_ticks();
    fn(color);
    unsigned long elapsed = timer_get_ticks() - start;
    printf("\n%10ld ticks %s(%s)\n", elapsed, f_name, c_name);
}

#define TIME_TRIAL(fn, color) _time_trial(#fn, fn, #color, color)

// -------------------------------------------------------------------
// Baseline redraw0, using public gl_xxx functions
// -------------------------------------------------------------------

static void redraw0(color_t color) {
    for (int y = 0; y < gl_get_height(); y++) {
        for (int x = 0; x < gl_get_width(); x++)  {
            gl_draw_pixel(x, y, color);
        }
    }
}

// -------------------------------------------------------------------
// redraw1, no gl_draw_pixel, write BGRA values directly into framebuffer
// -------------------------------------------------------------------

static void redraw1(color_t color) {
    uint8_t *ptr = fb_get_draw_buffer();
    for (int i = 0; i < gl_get_width() * gl_get_height(); i++) {
        *ptr++ = color & 0xff;           // blue
        *ptr++ = (color >> 8) & 0xff;    // green
        *ptr++ = (color >> 16) & 0xff;   // red
        *ptr++ = 0xff;                   // alpha
    }
}

// -------------------------------------------------------------------
// Improved redrawN functions of your own go here:
// -------------------------------------------------------------------






void main(void)  {
    timer_init();
    uart_init();
    gl_init(1280, 720, GL_SINGLEBUFFER);
    gl_clear(0xff666666);

    TIME_TRIAL(redraw0, GL_WHITE);
    TIME_TRIAL(redraw1, GL_RED);
    // TODO: Add more TIME_TRIAL calls here for your improved versions

    timer_delay(2);
}
