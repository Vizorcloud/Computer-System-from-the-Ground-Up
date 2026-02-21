/* File: fb.c
 * ----------
 * ***** TODO: add your file header comment here *****
 */
#include "fb.h"
#include "de.h"
#include "hdmi.h"
#include "malloc.h"
#include "strings.h"

// module-level variables, you may add/change this struct as you see fit
typedef struct {
    int width;             // count of horizontal pixels
    int height;            // count of vertical pixels
    int depth;             // num bytes per pixel
    void *framebuffer;     // address of framebuffer memory
} Module;

static Module on_screen_framebuffer = {0};
static Module off_screen_framebuffer = {0};
static fb_mode_t currentMode;

void fb_init(int width, int height, fb_mode_t mode) {
    currentMode = mode;

    // Free memory if already exists
    free(on_screen_framebuffer.framebuffer);
    on_screen_framebuffer.framebuffer = NULL;

    free(off_screen_framebuffer.framebuffer);
    off_screen_framebuffer.framebuffer = NULL;

    on_screen_framebuffer.width = width;
    on_screen_framebuffer.height = height;
    on_screen_framebuffer.depth = 4;

    int nbytes = on_screen_framebuffer.width * on_screen_framebuffer.height * on_screen_framebuffer.depth;
    on_screen_framebuffer.framebuffer = malloc(nbytes);
    memset(on_screen_framebuffer.framebuffer, 0x0, nbytes);
    
    if (mode == FB_DOUBLEBUFFER) {
        off_screen_framebuffer.width = width;
        off_screen_framebuffer.height = height;
        off_screen_framebuffer.depth = 4;

        off_screen_framebuffer.framebuffer = malloc(nbytes);
        memset(off_screen_framebuffer.framebuffer, 0x0, nbytes);
    }

    hdmi_resolution_id_t id = hdmi_best_match(width, height);
    hdmi_init(id);
    de_init(width, height, hdmi_get_screen_width(), hdmi_get_screen_height());
    de_set_active_framebuffer(on_screen_framebuffer.framebuffer);
}

int fb_get_width(void) {
    return on_screen_framebuffer.width;
}

int fb_get_height(void) {
    return on_screen_framebuffer.height;
}

int fb_get_depth(void) {
    return on_screen_framebuffer.depth;
}

void* fb_get_draw_buffer(void) {
    if (currentMode == FB_DOUBLEBUFFER) {
        return off_screen_framebuffer.framebuffer;
    }
    return on_screen_framebuffer.framebuffer;
}

void fb_swap_buffer(void) {
    if (currentMode != FB_DOUBLEBUFFER) return;

    void *temp = on_screen_framebuffer.framebuffer;
    on_screen_framebuffer.framebuffer = off_screen_framebuffer.framebuffer;
    off_screen_framebuffer.framebuffer = temp;

    de_set_active_framebuffer(on_screen_framebuffer.framebuffer);
}
