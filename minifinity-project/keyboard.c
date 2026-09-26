/* File: keyboard.c
 * -----------------
 * Purpose: Keyboard Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 17 2025
 */
#include "keyboard.h"
#include "ps2.h"
#include "malloc.h"

keyboard_t *keyboard_init(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    keyboard_t *keyboard = malloc(sizeof(*keyboard));
    keyboard->dev = ps2_new(clock_gpio, data_gpio);
    keyboard->modifiers = 0;
    keyboard->caps_pressed_last = false;
    return keyboard;
}

bool is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// returns true and sets c if a full keypress is ready, false otherwise
bool keyboard_try_read_next(keyboard_t *kb, char *c) {
    if (!ps2_has_data(kb->dev)) return false;
    
    uint8_t scan = ps2_read(kb->dev);
    
    if (scan == 0xE0) { kb->in_e0 = true; return false; }
    if (scan == 0xF0) { kb->in_f0 = true; return false; }
    
    // now we have the actual keycode
    key_action_t action;
    action.what = kb->in_f0 ? KEY_RELEASE : KEY_PRESS;
    action.keycode = scan;
    kb->in_e0 = false;
    kb->in_f0 = false;
    
    // run through your modifier logic
    switch (ps2_keys[scan].ch) {
        case PS2_KEY_SHIFT:
            if (action.what == KEY_PRESS) kb->modifiers |= KEYBOARD_MOD_SHIFT;
            else kb->modifiers &= ~KEYBOARD_MOD_SHIFT;
            return false;
        case PS2_KEY_ALT:
            if (action.what == KEY_PRESS) kb->modifiers |= KEYBOARD_MOD_ALT;
            else kb->modifiers &= ~KEYBOARD_MOD_ALT;
            return false;
        case PS2_KEY_CTRL:
            if (action.what == KEY_PRESS) kb->modifiers |= KEYBOARD_MOD_CTRL;
            else kb->modifiers &= ~KEYBOARD_MOD_CTRL;
            return false;
        case PS2_KEY_CAPS_LOCK:
            if (action.what == KEY_PRESS && !kb->caps_pressed_last)
                kb->modifiers ^= KEYBOARD_MOD_CAPS_LOCK;
            kb->caps_pressed_last = (action.what == KEY_PRESS);
            return false;
    }
    
    if (action.what == KEY_RELEASE) return false; // ignore releases
    
    // build the character
    char ch = ps2_keys[scan].ch;
    if (is_alpha(ch)) {
        if (kb->modifiers & KEYBOARD_MOD_CTRL) { *c = ch - 96; return true; }
        if (kb->modifiers & KEYBOARD_MOD_CAPS_LOCK) ch = ps2_keys[scan].other_ch;
    }
    if (kb->modifiers & KEYBOARD_MOD_SHIFT) ch = ps2_keys[scan].other_ch;
    *c = ch;
    return true;
}

uint8_t keyboard_read_scancode(keyboard_t *keyboard) {
    return ps2_read(keyboard->dev);
}

key_action_t keyboard_read_sequence(keyboard_t *keyboard) {
    uint8_t code = ps2_read(keyboard->dev);
    key_action_t action;

    // Handle extended prefix
    if (code == 0xE0) {
        code = ps2_read(keyboard->dev);
    }

    // Check if key was released
    if (code == 0xF0) {
        action.what = KEY_RELEASE;
        code = ps2_read(keyboard->dev);        
    } else {
        action.what = KEY_PRESS;
    }

    action.keycode = code; // store the final keycode

    return action;
}

key_event_t keyboard_read_event(keyboard_t *keyboard) {
    while (true) {
        key_action_t action = keyboard_read_sequence(keyboard);
        // Modifier Change logic
        switch (ps2_keys[action.keycode].ch) {
            case PS2_KEY_SHIFT:
                if (action.what == KEY_PRESS) { 
                    keyboard->modifiers |= KEYBOARD_MOD_SHIFT;
                } else {
                    keyboard->modifiers &= ~KEYBOARD_MOD_SHIFT;
                }
                continue;
            case PS2_KEY_ALT:
                if (action.what == KEY_PRESS) { 
                    keyboard->modifiers |= KEYBOARD_MOD_ALT;
                } else {
                    keyboard->modifiers &= ~KEYBOARD_MOD_ALT;
                }
                continue;
            case PS2_KEY_CTRL:
                if (action.what == KEY_PRESS) { 
                    keyboard->modifiers |= KEYBOARD_MOD_CTRL;
                } else { 
                    keyboard->modifiers &= ~KEYBOARD_MOD_CTRL;
                }
                continue;
            case PS2_KEY_CAPS_LOCK:
                if (action.what == KEY_PRESS && !(keyboard->caps_pressed_last)) { 
                    keyboard->modifiers ^= KEYBOARD_MOD_CAPS_LOCK; // toggle
                }
                keyboard->caps_pressed_last = (action.what == KEY_PRESS);
                continue;
        }
        
        // Non-Modifier Event Logic        
        key_event_t event;
        event.action = action;
        event.key = ps2_keys[action.keycode];
        event.modifiers = keyboard->modifiers; 

        return event;
    }
}

char keyboard_read_next(keyboard_t *keyboard) {
    key_event_t event = keyboard_read_event(keyboard);
    
    while (event.action.what == KEY_RELEASE) {
        event = keyboard_read_event(keyboard);
    }
    
    char character = event.key.ch;
    
    // Caps lock
    if (is_alpha(event.key.ch)) {  
        // Control Check
        bool controlEnabled = ((event.modifiers >> 2) & 0x1) == 1;        
        if (controlEnabled) return character - 96;

        character = ((event.modifiers >> 3) & 0x1) == 1 ? event.key.other_ch : character;
    }
    
    // Shift
    character = (event.modifiers & 0x1) == 1 ? event.key.other_ch : character;

    return character;
}