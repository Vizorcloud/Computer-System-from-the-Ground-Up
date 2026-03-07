/* File: console.c
 * -----------------
 * Purpose: Console Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 26 2025
 */

#include "console.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "malloc.h"
#include <stdarg.h>

static struct {
    int nrows;
    int ncols;
    int cursor_row;
    int cursor_col;
    color_t fg_color;
    color_t bg_color;
    int line_height;
    char *chars;
} module;

static void scroll_up(void);
static void advance_cursor(void);
static void put_char(char ch);
static void redraw(void);

void console_init(int nrows, int ncols, color_t foreground, color_t background) {
    const static int LINE_SPACING = 5;
    gl_init(ncols * gl_get_char_width(), nrows * (gl_get_char_height() + LINE_SPACING), GL_DOUBLEBUFFER);

    module.nrows = nrows;
    module.ncols = ncols;
    module.cursor_row = 0;
    module.cursor_col = 0;
    module.fg_color = foreground;
    module.bg_color = background;
    module.line_height = gl_get_char_height() + LINE_SPACING;

    free(module.chars);
    module.chars = malloc(nrows * ncols);

    console_clear();
}

void console_clear(void) {
    memset(module.chars, ' ', module.nrows * module.ncols);
    module.cursor_row = 0;
    module.cursor_col = 0;
    redraw();
}

int console_printf(const char *format, ...) {
    char buffer[1024];

    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    for (int i = 0; i < len; i++) {
        put_char(buffer[i]); 
    }

    redraw();
    return len;
}

static void put_char(char ch) {
    switch (ch) {
        case '\n':
            module.cursor_col = 0;
            module.cursor_row++;
            if (module.cursor_row >= module.nrows) {
                scroll_up();
                module.cursor_row = module.nrows - 1;
            }
            break;
        case '\b':
            if (module.cursor_col > 0) {
                module.cursor_col--;
                module.chars[module.cursor_row * module.ncols + module.cursor_col] = ' ';
            } else if (module.cursor_row > 0) {
                module.cursor_row--;
                module.cursor_col = module.ncols - 1;
                module.chars[module.cursor_row * module.ncols + module.cursor_col] = ' ';
            }
            break;
        case '\f':
            console_clear();
            break;
        default:
            module.chars[module.cursor_row * module.ncols + module.cursor_col] = ch;
            advance_cursor();
            break;
    }
}

static void advance_cursor(void) {
    module.cursor_col++;
    if (module.cursor_col >= module.ncols) {
        module.cursor_col = 0;
        module.cursor_row++;
        if (module.cursor_row >= module.nrows) {
            scroll_up();
            module.cursor_row = module.nrows - 1;
        }
    }
}

static void scroll_up(void) {
    // move all rows up by one
    for (int r = 1; r < module.nrows; r++) {
        memcpy(
            &module.chars[(r-1)*module.ncols],
            &module.chars[r*module.ncols],
            module.ncols
        );
    }

    // clear the bottom row so next line prints there
    memset(&module.chars[(module.nrows-1)*module.ncols], ' ', module.ncols);

    // cursor stays at bottom row
    module.cursor_row = module.nrows - 1;
}

static void redraw(void) {
    gl_clear(module.bg_color);

    int char_w = gl_get_char_width();

    for (int r = 0; r < module.nrows; r++) {
        for (int c = 0; c < module.ncols; c++) {
            char ch = module.chars[r * module.ncols + c];
            if (ch != ' ') {
                gl_draw_char(c * char_w, r * module.line_height, ch, module.fg_color);
            }
        }
    }

    gl_swap_buffer();
}

void console_startup_screen(void) { }