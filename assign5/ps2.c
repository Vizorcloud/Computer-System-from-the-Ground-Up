/* File: ps2_assign5.c
 * -------------------
 * Purpose: Ps2 Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 17 2025
 */
#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"
#include "timer.h"

#define TICKS_PER_USEC 24
#define PS2_BIT_MAX_GAP_US 500

typedef struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
    unsigned int last_edge;
} ps2_device_t;

typedef struct scan_code {
    unsigned int startBit;
    unsigned int dataNum;
    unsigned int parityBit;
    unsigned int stopBit;
    int onesCount;
} scan_code;

ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    ps2_device_t *dev = malloc(sizeof(*dev));
    dev->clock = clock_gpio;
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);
    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);
    dev->last_edge = 0;
    return dev;
}

// Waits for falling clock edge, reads data bit.
// Returns false if gap detected (timeout), true otherwise.
// Always writes the bit value to *bit.
bool read_bit(ps2_device_t *dev, int *bit) {
    while (gpio_read(dev->clock) == 0);
    while (gpio_read(dev->clock) == 1);

    unsigned int now = timer_get_ticks() / TICKS_PER_USEC;
    unsigned int last = dev->last_edge;
    dev->last_edge = now;

    *bit = gpio_read(dev->data);

    if (last && (now - last) > PS2_BIT_MAX_GAP_US) {
        return false;  // gap detected, caller should treat this as a new start bit
    }
    return true;
}

bool is_parity_valid(scan_code code) {
    return ((code.onesCount + code.parityBit) % 2) == 1;
}

scan_code read_scancode(ps2_device_t *dev) {
    scan_code scannedCode;
    int bit;

    while (true) {
        // wait for start bit (must be 0)
        if (!read_bit(dev, &bit)) {
            if (bit == 0) goto got_start;  // gap bit is the new start bit
            continue;
        }
        if (bit != 0) continue;

    got_start:;
        int onesCount = 0;
        uint8_t data = 0;

        // DATA BITS
        bool failed = false;
        for (int i = 0; i < 8; i++) {
            if (!read_bit(dev, &bit)) { failed = true; break; }
            data |= (bit & 1) << i;
            onesCount += (bit & 1);
        }
        if (failed) {
            if (bit == 0) goto got_start;
            continue;
        }

        // PARITY BIT
        if (!read_bit(dev, &bit)) {
            if (bit == 0) goto got_start;
            continue;
        }
        int parityBit = bit;

        // STOP BIT
        if (!read_bit(dev, &bit)) {
            if (bit == 0) goto got_start;
            continue;
        }
        int stopBit = bit;

        scannedCode.startBit = 0;
        scannedCode.dataNum = data;
        scannedCode.parityBit = parityBit;
        scannedCode.stopBit = stopBit;
        scannedCode.onesCount = onesCount;

        if (!is_parity_valid(scannedCode)) continue;  // retry, no resync needed
        if (scannedCode.stopBit != 1) continue;        // retry, no resync needed

        return scannedCode;
    }
}

uint8_t ps2_read(ps2_device_t *dev) {
    scan_code readCode = read_scancode(dev);
    return readCode.dataNum;
}