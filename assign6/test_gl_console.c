/* File: test_gl_console.c
 * -----------------------
 * Purpose: Console related modules testing
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 26 2025
 */
#include "assert.h"
#include "console.h"
#include "fb.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"
#include "uart.h"

static void pause(const char *message) {
    if (message) printf("\n%s\n", message);
    printf("[PAUSED] type any key in tio to continue: ");
    int ch = uart_getchar();
    uart_putchar(ch);
    uart_putchar('\n');
}

static void test_fb(void) {
    const int SIZE = 500;
    fb_init(SIZE, SIZE, FB_SINGLEBUFFER); // init single buffer

    assert(fb_get_width() == SIZE);
    assert(fb_get_height() == SIZE);
    assert(fb_get_depth() == 4);

    void *cptr = fb_get_draw_buffer();
    assert(cptr != NULL);
    int nbytes = fb_get_width() * fb_get_height() * fb_get_depth();
    memset(cptr, 0x99, nbytes); // fill entire framebuffer with light gray pixels
    pause("Now displaying 500 x 500 screen of light gray pixels");

    fb_init(1280, 720, FB_DOUBLEBUFFER); // init double buffer
    cptr = fb_get_draw_buffer();
    nbytes =  fb_get_width() * fb_get_height() * fb_get_depth();
    memset(cptr, 0xff, nbytes); // fill one buffer with white pixels
    fb_swap_buffer();
    pause("Now displaying 1280 x 720 white pixels");

    cptr = fb_get_draw_buffer();
    memset(cptr, 0x33, nbytes); // fill other buffer with dark gray pixels
    fb_swap_buffer();
    pause("Now displaying 1280 x 720 dark gray pixels");

    for (int i = 0; i < 5; i++) {
        fb_swap_buffer();
        timer_delay_ms(250);
    }
}

static void test_gl(void) {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    // Double buffer mode
    gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);
    assert(gl_get_height() == HEIGHT);
    assert(gl_get_width() == WIDTH);

    // Background is purple
    gl_clear(gl_color(0x55, 0, 0x55)); // create purple color

    // Draw green pixel in lower right
    gl_draw_pixel(WIDTH-10, HEIGHT-10, GL_GREEN);
    assert(gl_read_pixel(WIDTH-10, HEIGHT-10) == GL_GREEN);

    // Blue rectangle in center of screen
    gl_draw_rect(WIDTH/2 - 100, HEIGHT/2 - 50, 200, 100, GL_BLUE);

    // Single amber character
    gl_draw_char(60, 10, 'A', GL_AMBER);

    // Show buffer with drawn contents
    gl_swap_buffer();
    pause("Now displaying 1280 x 720, purple bg, single green pixel, blue center rect, amber letter A");
}

static void student_test_gl_singlebuffer(void) {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    // Single buffer mode
    gl_init(WIDTH, HEIGHT, GL_SINGLEBUFFER);
    //gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);
    assert(gl_get_height() == HEIGHT);
    assert(gl_get_width() == WIDTH);

    // Background is purple
    gl_clear(gl_color(0x55, 0, 0x55));

    // Draw green pixel in lower right
    gl_draw_pixel(WIDTH - 10, HEIGHT - 10, GL_GREEN);
    assert(gl_read_pixel(WIDTH - 10, HEIGHT - 10) == GL_GREEN);

    // Blue rectangle in center
    gl_draw_rect(WIDTH/2 - 100, HEIGHT/2 - 50, 200, 100, GL_BLUE);

    // Amber character
    gl_draw_char(60, 10, 'A', GL_AMBER);
    
    for (int i = 0; i < 2000; i++) {
        gl_clear(GL_BLACK);

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if ((x + y + i) % 50 == 0) {
                    gl_draw_pixel(x, y, GL_GREEN);
                }
            }
        }

        gl_swap_buffer();
    }

    // In single buffer mode, drawing should already be visible.
    // This should do nothing
    gl_swap_buffer();

    pause("Now displaying 800 x 600 in SINGLE BUFFER mode");
}

static void student_test_init_and_getters(void) {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    gl_init(WIDTH, HEIGHT, GL_SINGLEBUFFER);
    assert(gl_get_width() == WIDTH);
    assert(gl_get_height() == HEIGHT);

    gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);
    assert(gl_get_width() == WIDTH);
    assert(gl_get_height() == HEIGHT);

    gl_swap_buffer();
    pause("student_test_init_and_getters passed!");
}

static void student_test_gl_color(void) {
    color_t c = gl_color(0x55, 0x00, 0x55); // purple
    
    assert(c == ((0xFF << 24) | (0x55 << 16) | (0x00 << 8) | 0x55));
    printf("student_test_gl_color passed!\n");
}

static void student_test_gl_clear(void) {
    const int WIDTH = gl_get_width();
    const int HEIGHT = gl_get_height();

    gl_clear(GL_BLACK);

    // Check corners and center
    assert(gl_read_pixel(0, 0) == GL_BLACK);
    assert(gl_read_pixel(WIDTH / 2, HEIGHT / 2) == GL_BLACK);
    assert(gl_read_pixel(WIDTH - 1, HEIGHT - 1) == GL_BLACK);
    
    gl_swap_buffer();
    pause("student_test_gl_clear passed 1/2");

    gl_clear(GL_WHITE);
    assert(gl_read_pixel(0, 0) == GL_WHITE);
    assert(gl_read_pixel(WIDTH / 2, HEIGHT / 2) == GL_WHITE);
    assert(gl_read_pixel(WIDTH - 1, HEIGHT - 1) == GL_WHITE);

    gl_swap_buffer();
    pause("student_test_gl_clear passed! 2/2");
}

static void student_test_gl_draw_pixel(void) {
    const int WIDTH = gl_get_width();
    const int HEIGHT = gl_get_height();

    gl_clear(GL_BLACK);

    // Draw pixels inside the screen
    gl_draw_pixel(10, 10, GL_GREEN);
    gl_draw_pixel(WIDTH - 1, HEIGHT - 1, GL_RED);

    assert(gl_read_pixel(10, 10) == GL_GREEN);
    assert(gl_read_pixel(WIDTH - 1, HEIGHT - 1) == GL_RED);

    gl_swap_buffer();
    pause("student_test_gl_draw_pixel passed! 1/2");

    gl_clear(GL_BLACK);

    // Draw pixels outside the screen should have no effect
    gl_draw_pixel(-1, 0, GL_BLUE);
    gl_draw_pixel(0, -1, GL_BLUE);
    gl_draw_pixel(WIDTH, 0, GL_BLUE);
    gl_draw_pixel(0, HEIGHT, GL_BLUE);

    assert(gl_read_pixel(0, 0) == GL_BLACK);
    assert(gl_read_pixel(WIDTH - 2, HEIGHT - 2) == GL_BLACK);

    gl_swap_buffer();
    pause("student_test_gl_draw_pixel passed! 2/2");
}

static void student_test_gl_draw_rect(void) {
    const int WIDTH = gl_get_width();
    const int HEIGHT = gl_get_height();

    gl_clear(GL_BLACK);

    // Normal rectangle
    gl_draw_rect(5, 5, 10, 10, GL_BLUE);
    for (int y = 5; y < 15; y++) {
        for (int x = 5; x < 15; x++) {
            assert(gl_read_pixel(x, y) == GL_BLUE);
        }
    }

    gl_swap_buffer();
    pause("student_test_gl_draw_rect passed! 1/3");

    // Rectangle partially off-screen (top-left)
    gl_draw_rect(-5, -5, 10, 10, GL_RED);
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            assert(gl_read_pixel(x, y) == GL_RED);
        }
    }

    gl_swap_buffer();
    pause("student_test_gl_draw_rect passed! 2/3");

    // Rectangle partially off-screen (bottom-right)
    gl_draw_rect(WIDTH - 5, HEIGHT - 5, 10, 10, GL_GREEN);
    for (int y = HEIGHT - 5; y < HEIGHT; y++) {
        for (int x = WIDTH - 5; x < WIDTH; x++) {
            assert(gl_read_pixel(x, y) == GL_GREEN);
        }
    }

    gl_swap_buffer();
    pause("student_test_gl_draw_rect passed! 3/3");
}

static void student_test_char(void) {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    gl_init(WIDTH, HEIGHT, GL_DOUBLEBUFFER);

    // Purple background
    gl_clear(gl_color(0x55, 0x00, 0x55));

    // Centered congratulatory message
    gl_draw_string(200, 200, "CONGRATULATIONS!", GL_AMBER);
    gl_draw_string(220, 240, "Your text rendering works.", GL_AMBER);

    // Test newline handling
    gl_draw_string(200, 300, "Line 1\nLine 2\nLine 3", GL_AMBER);

    // Clipping tests

    // Partially off left edge
    gl_draw_string(-20, 350, "LEFT CLIP", GL_AMBER);

    // Partially off right edge
    gl_draw_string(WIDTH - 30, 380, "RIGHT CLIP", GL_AMBER);

    // Partially off top edge
    gl_draw_string(300, -10, "TOP CLIP", GL_AMBER);

    // Partially off bottom edge
    gl_draw_string(300, HEIGHT - 5, "BOTTOM CLIP", GL_AMBER);

    gl_swap_buffer();

    pause("Displaying congratulatory message and clipping tests");
}

static void test_console(void) {
    gl_init(800, 600, GL_DOUBLEBUFFER);
    console_init(25, 50, GL_CYAN, GL_INDIGO);
    pause("Now displaying console: 25 rows x 50 columns, bg indigo, fg cyan");

    // Line 1: Hello, world!
    console_printf("Hello, world!\n");

    // Add line 2: Happiness == CODING
    console_printf("Happiness");
    console_printf(" == ");
    console_printf("CODING\n");

    // Add 2 blank lines and line 5: I am Pi, hear me roar!
    console_printf("\n\nI am Pi, hear me v\b \broar!\n"); // typo, backspace, correction
    pause("Console printfs");

    // Clear all lines
    console_printf("\f");

    // Line 1: "Goodbye"
    console_printf("Goodbye!\n");
    pause("Console clear");
}

static void student_test_console(void) {
    // Initialize a small console so scrolling is obvious
    console_init(7, 20, GL_CYAN, GL_BLACK); // make the console taller
    pause("Console initialized: 5 rows x 20 cols");

    // Normal printing
    console_printf("Line 1\n");
    console_printf("Line 2\n");
    console_printf("Line 3\n");
    pause("After 3 lines");

    // Horizontal wrapping 
    console_printf("This line is longer than twenty chars\n");
    pause("After long line, should wrap horizontally");

    // Backspace 
    console_printf("Typing errr\b\b\b\bcorrect\n");
    pause("Backspace test: 'errr' corrected to 'correct'");

    // Form feed (clear screen) 
    console_printf("\f");
    console_printf("Screen cleared!\n");
    pause("Form feed test");

    // Vertical scrolling 
    for (int i = 1; i <= 6; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Line %d\n", i);
        console_printf(buf);
    }
    pause("Vertical scroll test: top lines should have scrolled off");

    // Mixed 
    console_printf("Mixing \b\b\b\bbackspace, wrap and scroll\n");
    console_printf("Final line to scroll\n");
    pause("End of mixed test");
}

void main(void) {
    timer_init();
    uart_init();
    printf("Executing main() in test_gl_console.c\n");

    //test_fb();
    // test_gl();
    // student_test_gl_singlebuffer();
    // student_test_init_and_getters();
    // student_test_gl_color();
    // student_test_gl_clear();
    // student_test_gl_draw_pixel();
    // student_test_gl_draw_rect();
    //test_console();
    student_test_console();
    // student_test_char();

    printf("Completed main() in test_gl_console.c\n");
}
