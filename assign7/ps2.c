/* File: ps2.c
 * -----------
 * Purpose: PS2 Interrupts Module Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Mar 5 2025
 */
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupt.h"
#include "malloc.h"
#include "ringbuffer.h"
#include "ps2.h"
#include "timer.h"

#define TICKS_PER_USEC 24 // 24 ticks counted per one microsecond
#define PS2_BIT_MAX_GAP_US 500
#define PS2_TIMEOUT_TICKS (PS2_BIT_MAX_GAP_US * TICKS_PER_USEC)  // 500 * 24 = 12000

typedef struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
    int bit_count;        // 0=start, 1-8=data, 9=parity, 10=stop
    uint8_t data_byte;    // accumulates data bits
    int ones_count;       // count of 1-bits for parity check
    rb_t *scancode_queue;
    uint32_t last_bit_time;   // <-- ADD THIS
} ps2_device_t;

static void ps2_handler(void *aux_data) {
    ps2_device_t *dev = aux_data;

    // Check for timeout — if gap too long, discard partial scancode
    uint32_t now = timer_get_ticks();
    if (dev->bit_count > 0 && (now - dev->last_bit_time) > PS2_TIMEOUT_TICKS) {
        dev->bit_count  = 0;
        dev->data_byte  = 0;
        dev->ones_count = 0;
    }
    dev->last_bit_time = now;   // update AFTER the check

    int bit = gpio_read(dev->data);

    switch (dev->bit_count) {
        case 0: // START bit — must be 0
            if (bit != 0) {
                // bad start bit, stay at 0, don't advance
                gpio_interrupt_clear(dev->clock);
                return;
            }
            dev->bit_count++;
            break;

        case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8: // DATA bits, LSB first
            dev->data_byte |= (bit << (dev->bit_count - 1));
            dev->ones_count += bit;
            dev->bit_count++;
            break;

        case 9: // PARITY bit — odd parity, ones_count+parity must be odd
            if ((dev->ones_count + bit) % 2 != 1) {
                // parity error, reset
                dev->bit_count = 0;
                dev->data_byte = 0;
                dev->ones_count = 0;
                gpio_interrupt_clear(dev->clock);
                return;
            }
            dev->bit_count++;
            break;

        case 10: // STOP bit — must be 1
            if (bit == 1) {
                rb_enqueue(dev->scancode_queue, dev->data_byte);
            }
            // reset regardless
            dev->bit_count = 0;
            dev->data_byte = 0;
            dev->ones_count = 0;
            break;
    }

    gpio_interrupt_clear(dev->clock);
}

ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    ps2_device_t *dev = malloc(sizeof(*dev));
    dev->clock = clock_gpio;
    dev->data  = data_gpio;

    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);

    dev->bit_count  = 0;
    dev->data_byte  = 0;
    dev->ones_count = 0;
    dev->scancode_queue = rb_new();
    dev->last_bit_time = 0;

    // gpio-level interrupt setup (local to this device)
    gpio_interrupt_init();
    gpio_interrupt_config(dev->clock, GPIO_INTERRUPT_NEGATIVE_EDGE, false);
    gpio_interrupt_register_handler(dev->clock, ps2_handler, dev);
    gpio_interrupt_enable(dev->clock);

    return dev;
}

uint8_t ps2_read(ps2_device_t *dev) {
    while (rb_empty(dev->scancode_queue)) ; // spin until scancode ready
    return (uint8_t)rb_dequeue(dev->scancode_queue);
}