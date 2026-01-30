/* File: clock.c
 * --------------
 * Purpose: Clock library for managing time on the Mango Pi
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Jan 27 2025
 */

#include "gpio.h"
#include "gpio_extra.h"
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

static const int8_t enc_table[16] = {
     0, -1, +1,  0,
    +1,  0,  0, -1,
    -1,  0,  0, +1,
     0, +1, -1,  0
};

#define LATCH0 0b00
#define LATCH3 0b11

int wait_for_button(void) {
    int duration = DURATION;      // starting value
    uint8_t digits[4];

    int last_state = (gpio_read(GPIO_PG13) << 1) | gpio_read(GPIO_PG12);
    int position = 0;             // micro-step counter
    int lastDetent = 0;           // last full detent value

    while (gpio_read(GPIO_PD12) == 1) { // while button not pressed
        int A = gpio_read(GPIO_PG13);
        int B = gpio_read(GPIO_PG12);
        int state = (A << 1) | B;

        // micro-step delta
        int delta = enc_table[(last_state << 2) | state];
        last_state = state;

        if (delta != 0) {
            position += delta;   // accumulate micro-steps

            // Only update duration at a latch state
            if (state == LATCH0 || state == LATCH3) {
                int detent = position >> 1; // full detent
                int diff = detent - lastDetent;

                if (diff != 0) {
                    duration += diff;      // increment or decrement by number of detents
                    if (duration < 0) duration = 0;
                    lastDetent = detent;  // update last detent
                }
            }
        }

        // refresh display
        static int counter = 0;
        counter++;
        if (counter >= 5) {
            seconds_to_digits(duration, digits);
            refresh_display(digits, 5);
            counter = 0;
        }

        timer_delay_us(500);
    }
    return duration;
}

void run_countdown(int duration) {
    uint8_t digits[4];
    int remaining = duration;

    while (remaining >= 0) {
        seconds_to_digits(remaining, digits);

        // Refresh display many times in a loop over 1 second
        for (int i = 0; i < 50; i++) {
            refresh_display(digits, 5); // 2 ms per digit
        }

        remaining--; // decrement 1 second
    }
}

void countdown_finished(void) {
    for (int i = 0; i < 10; i++) { // blink 10 times
        // Fade LED on
        for (int step = 0; step <= 5; step++) {
            for (int d = 0; d < 4; d++) {
                display_digit_at(d, 8); // keep digits visible
            }
            gpio_write(GPIO_PB5, 1);       // blue LED on
            timer_delay_ms(50);       // short step delay
            gpio_write(GPIO_PB5, 0);     // LED off to simulate fading
            timer_delay_ms(50);
        }

        // Delay to match original blink timing
        disable_all_digits();
        timer_delay_ms(500);
    }

    gpio_write(GPIO_PB1, 0); // ensure LED off at end
}

int main(void) {
    int countdown = DURATION;
        
    gpio_set_input(GPIO_PD12); 
    gpio_set_input(GPIO_PG13);     
    gpio_set_input(GPIO_PG12);
    gpio_set_pullup(GPIO_PD12); 
    gpio_set_pullup(GPIO_PG13);
    gpio_set_pullup(GPIO_PG12);
    gpio_set_output(GPIO_PB5);
    gpio_set_output(GPIO_PB1);    
    gpio_set_output(GPIO_PD14);
    gpio_write(GPIO_PB1, 1);    

    initialize_pins();
    disable_all_digits();

    timer_delay_ms(20);   
    countdown = wait_for_button();    // wait until user presses button
    gpio_write(GPIO_PB1, 0);
    gpio_write(GPIO_PD14, 1);
    run_countdown(countdown);         // countdown from DURATION seconds
    gpio_write(GPIO_PD14, 0);
    countdown_finished();            // signal end of countdown
                                            
    return countdown;
}
