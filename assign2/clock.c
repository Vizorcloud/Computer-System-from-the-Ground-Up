/* File: clock.c
 * --------------
 * Purpose: Clock library for managing time on the Mango Pi
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Jan 27 2025
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

void disable_all_digits(void) {
    for (int d = 0; d < 4; d++) {
        gpio_write(digit_pins[d], 0);
    }
}

void display_digit_at(uint8_t digitIndex, uint8_t digit) {
    if (digitIndex > 3 || digit > 9) return;

    display_digit(digit);
    
    gpio_write(digit_pins[digitIndex], 1);
}

void refresh_display(uint8_t displayedDigits[4], int refreshRate) {
    for (int pos = 0; pos < 4; pos++) {
        disable_all_digits();
        display_digit_at(pos, displayedDigits[pos]);
        timer_delay_ms(refreshRate);  // 1 ms per digit
    }
}

void seconds_to_digits(int totalSeconds, uint8_t digits[4]) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    digits[0] = minutes / 10;   // tens of minutes
    digits[1] = minutes % 10;   // ones of minutes
    digits[2] = seconds / 10;   // tens of seconds
    digits[3] = seconds % 10;   // ones of seconds
}

void wait_for_button(void) {
    while (gpio_read(GPIO_PD12) == 1) {
        // idle until button is pressed
        // keep refreshing display if you want idle pattern
    }
}

void run_countdown(int duration) {
    uint8_t digits[4];
    int remaining = duration;

    while (remaining >= 0) {
        seconds_to_digits(remaining, digits);

        // Refresh display many times in a loop over 1 second
        int loops = 1000 / 2; // 2 ms per digit
        for (int i = 0; i < 50; i++) {
            refresh_display(digits, 5); // 2 ms per digit
        }

        remaining--; // decrement 1 second
    }
}

void countdown_finished(void) {
    for (int i = 0; i < 10; i++) { // blink 10 times
        for (int d = 0; d < 4; d++) {
            display_digit_at(d, 8); // display '8' for full segments
        }
        timer_delay_ms(500);
        disable_all_digits();
        timer_delay_ms(500);
    }
}

int main(void) {
    int countdown = DURATION;
    
    gpio_set_input(GPIO_PD12); 

    initialize_pins();
    disable_all_digits();
    
    wait_for_button();               // wait until user presses button
    run_countdown(DURATION);         // countdown from DURATION seconds
    countdown_finished();            // signal end of countdown
                                           // 
    return countdown;
}
