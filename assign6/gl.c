/* File: gl.c
 * ----------
 * Purpose: Gl Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 26 2025
 */
#include "gl.h"
#include "font.h"
#include "gl_mine.h"

void gl_init(int width, int height, gl_mode_t mode) {
    fb_init(width, height, mode);
}

int gl_get_width(void) {
    return fb_get_width();
}

int gl_get_height(void) {
    return fb_get_height();
}

color_t gl_color(uint8_t r, uint8_t g, uint8_t b) {
    return (0xff << 24) | (r << 16) | (g << 8) | b;
}

void gl_swap_buffer(void) {
    fb_swap_buffer();
}

void gl_clear(color_t c) {
    int width = fb_get_width();
    int height = fb_get_height();
    unsigned int (*pixelGrid)[width] = fb_get_draw_buffer();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixelGrid[y][x] = c;
        }
    }
}

void gl_draw_pixel(int x, int y, color_t c) {
    int width = fb_get_width();
    int height = fb_get_height();

    if (x < 0 || x >= width || y < 0 || y >= height) return;

    unsigned int (*pixelGrid)[width] = fb_get_draw_buffer();
    pixelGrid[y][x] = c;
}

color_t gl_read_pixel(int x, int y) {
    int width = fb_get_width();
    int height = fb_get_height();
    unsigned int (*pixelGrid)[width] = fb_get_draw_buffer();

    // Returns black on invalid grab
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;

    color_t pixelColor = pixelGrid[y][x];
    return pixelColor;
}

void gl_draw_rect(int x, int y, int w, int h, color_t c) {
    int canvasWidth = fb_get_width();
    int canvasHeight = fb_get_height();
    unsigned int (*pixelGrid)[canvasWidth] = fb_get_draw_buffer();

    // Clip negative starts
    int xStart = x < 0 ? 0 : x;
    int yStart = y < 0 ? 0 : y;

    // Clip going off canvas
    int yEnd = (y + h > canvasHeight) ? canvasHeight : y + h;
    int xEnd = (x + w > canvasWidth) ? canvasWidth : x + w;

    for (int yCoord = yStart; yCoord < yEnd; yCoord++) {
        for (int xCoord = xStart; xCoord < xEnd; xCoord++) {
            pixelGrid[yCoord][xCoord] = c;
        }
    }
}

void gl_draw_char(int x, int y, char ch, color_t c) {
    int glyphHeight = gl_get_char_height();
    int glyphWidth = gl_get_char_width();
    int glyphSize = font_get_glyph_size();
    int canvasWidth = fb_get_width();
    int canvasHeight = fb_get_height();

    unsigned int (*pixelGrid)[canvasWidth] = fb_get_draw_buffer();

    uint8_t buf[glyphSize];

    if (!font_get_glyph(ch, buf, glyphSize)) return;

    for (int row = 0; row < glyphHeight; row++) {
        int screenY = y + row;

        if (screenY < 0 || screenY >= canvasHeight) continue;

        for (int col = 0; col < glyphWidth; col++) {
            int screenX = x + col;

            if (screenX < 0 || screenX >= canvasWidth) continue;

            if (buf[row * glyphWidth + col] == 0xFF) {
                pixelGrid[screenY][screenX] = c;
            }
        }
    }
}

void gl_draw_string(int x, int y, const char* str, color_t c) {
    int startX = x;
    int glyphWidth  = gl_get_char_width();
    int glyphHeight = gl_get_char_height();

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            x = startX;
            y += glyphHeight;
            continue;
        }

        gl_draw_char(x, y, str[i], c);
        x += glyphWidth;
    }
}

int gl_get_char_height(void) {
    return font_get_glyph_height();
}

int gl_get_char_width(void) {
    return font_get_glyph_width();
}