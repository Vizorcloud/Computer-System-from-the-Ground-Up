/* File: clock.c
 * -------------
 * ***** TODO: add your file header comment here *****
 */
#include "gpio.h"
#include "timer.h"
#include <stdint.h>

/* __Note__: Do not edit/remove the three lines of code below.
 * These preprocessor directives cooperate with Makefile to allow
 * setting DURATION as part of the build process. You can test your
 * clock application on the duration of your choice by specifying
 * value when invoking make like so
 *       make run DURATION=30
 * The value for DURATION is expressed in seconds and defaults to
 * 67 seconds if not explicitly set. (i.e. 1 min 7 seconds)
 */
#ifndef DURATION
#define DURATION 67
#endif

static const gpio_id_t segment_pins[] = {
    GPIO_PD17, // A
    GPIO_PB6, // B
    GPIO_PB12, // C
    GPIO_PB11, // D
    GPIO_PB10, // E
    GPIO_PD11, // F
    GPIO_PD13  // G
};

static const gpio_id_t digit_pins[] = {
    GPIO_PB4, // 1
    GPIO_PB3,  // 2
    GPIO_PB2, // 3
    GPIO_PC0 // 4
};

static const uint8_t display_digit_bits[] = {
    0b00111111, // 0: A B C D E F
    0b00000110, // 1: B C
    0b01011011, // 2: A B D E G
    0b01001111, // 3: A B C D G
    0b01100110, // 4: B C F G
    0b01101101, // 5: A C D F G
    0b01111101, // 6: A C D E F G
    0b00000111, // 7: A B C
    0b01111111, // 8: A B C D E F G
    0b01101111  // 9: A B C D F G
};

void display_digit(uint8_t digit) {
    if (digit > 9) return;

    uint8_t digitBitPattern = display_digit_bits[digit];

    for (int i = 0; i < 7; i++) {
        int bit = (digitBitPattern >> i) & 1;
        gpio_write(segment_pins[i], bit);
    }
}

void initialize_pins(void) {
    for (int i = 0; i < 7; i++) {
        gpio_set_output(segment_pins[i]);
    }
    for (int i = 0; i < 4; i++) {
        gpio_set_output(digit_pins[i]);
    }
}

int main(void) {
    int countdown = DURATION;
    
    initialize_pins(); 
    
    gpio_write(digit_pins[0], 1);
    
    while (1) {
        for (uint8_t d = 0; d < 10; d++) {
            display_digit(d);
            timer_delay_ms(500);
        }
    }

    return countdown;
}
