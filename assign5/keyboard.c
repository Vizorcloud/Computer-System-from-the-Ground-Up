/* File: keyboard.c
 * -----------------
 * Purpose: Keyboard Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 17 2025
 */
#include "keyboard.h"
#include "ps2.h"

static ps2_device_t *dev;

void keyboard_init(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    dev = ps2_new(clock_gpio, data_gpio);
}

uint8_t keyboard_read_scancode(void) {
    return ps2_read(dev);
}

key_action_t keyboard_read_sequence(void) {
    uint8_t code = ps2_read(dev);
    key_action_t action;

    // Handle extended prefix
    if (code == 0xE0) {
        code = ps2_read(dev);
    }

    // Check if key was released
    if (code == 0xF0) {
        action.what = KEY_RELEASE;
        code = ps2_read(dev);        
    } else {
        action.what = KEY_PRESS;
    }

    action.keycode = code; // store the final keycode

    return action;
}

key_event_t keyboard_read_event(void) {
    static keyboard_modifiers_t modifiers = 0;
    static bool caps_pressed_last = false;

    while (true) {
        key_action_t action = keyboard_read_sequence();
        // Modifier Change logic
        switch (ps2_keys[action.keycode].ch) {
            case PS2_KEY_SHIFT:
                if (action.what == KEY_PRESS) { 
                    modifiers |= KEYBOARD_MOD_SHIFT;
                } else {
                    modifiers &= ~KEYBOARD_MOD_SHIFT;
                }
                continue;
            case PS2_KEY_ALT:
                if (action.what == KEY_PRESS) { 
                    modifiers |= KEYBOARD_MOD_ALT;
                } else {
                    modifiers &= ~KEYBOARD_MOD_ALT;
                }
                continue;
            case PS2_KEY_CTRL:
                if (action.what == KEY_PRESS) { 
                    modifiers |= KEYBOARD_MOD_CTRL;
                } else { 
                    modifiers &= ~KEYBOARD_MOD_CTRL;
                }
                continue;
            case PS2_KEY_CAPS_LOCK:
                if (action.what == KEY_PRESS && !caps_pressed_last) { 
                    modifiers ^= KEYBOARD_MOD_CAPS_LOCK; // toggle
                }
                caps_pressed_last = (action.what == KEY_PRESS);
                continue;
        }
        
        // Non-Modifier Event Logic        
        key_event_t event;
        event.action = action;
        event.key = ps2_keys[action.keycode];
        event.modifiers = modifiers; 

        return event;
    }
}

bool is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

char keyboard_read_next(void) {
    key_event_t event = keyboard_read_event();
    
    while (event.action.what == KEY_RELEASE) {
        event = keyboard_read_event();
    }
    
    char character = event.key.ch;

    // Caps lock
    if (is_alpha(event.key.ch)) {  
        character = ((event.modifiers >> 3) & 0x1) == 1 ? event.key.other_ch : character;
    }
    
    // Shift
    character = (event.modifiers & 0x1) == 1 ? event.key.other_ch : character;

    return character;
}
