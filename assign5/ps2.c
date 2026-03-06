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

#define TICKS_PER_USEC 24 // 24 ticks counted per one microsecond
#define PS2_BIT_MAX_GAP_US 500
// A ps2_device is a structure that stores all of the state and information
// needed for a PS2 device. The clock field stores the gpio id for the
// clock pin, and the data field stores the gpio id for the data pin.
// Read ps2_new for example code that sets and uses these fields.
//
// You may extend the ps2_device structure with additional fields as needed.
// A pointer to the current ps2_device is passed into all ps2_ calls.
// Storing state in this structure is preferable to using global variables:
// it allows your driver to support multiple PS2 devices accessed concurrently
// (e.g., a keyboard and a mouse).
//
// This definition fills out the structure declared in ps2.h.
typedef struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
    unsigned int last_edge;  
    bool resync;  
} ps2_device_t;

// Creates a new PS2 device connected to given clock and data pins,
typedef struct scan_code {
    unsigned int startBit;
    unsigned int dataNum;
    unsigned int parityBit;
    unsigned int stopBit;
    int onesCount;
} scan_code;

// The gpios are configured as input and set to use internal pull-up
// (PS/2 protocol requires clock/data to be high default)
ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    // consider why must malloc be used to allocate device
    ps2_device_t *dev = malloc(sizeof(*dev));

    dev->clock = clock_gpio;
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);

    dev->last_edge = 0; // start fresh
    dev->resync = false;
    return dev;
}

// Returns true if successfully read a bit and false if desynchronization 
// necessitates a restart of the code
bool read_bit(ps2_device_t *dev, int *bit) {
    // Ensure we start HIGH before reading
    while (gpio_read(dev->clock) == 0);
    // Wait until the clock pin reads LOW
    while (gpio_read(dev->clock) == 1);
    
    unsigned int now = timer_get_ticks() / TICKS_PER_USEC;

    if (dev->resync && dev->last_edge && (now - dev->last_edge) > PS2_BIT_MAX_GAP_US) {
        dev->last_edge = now;
        dev->resync = false;
        return false;
    }
    dev->last_edge = now;
    *bit = gpio_read(dev->data);
    return true;    
}

bool is_parity_valid(scan_code code) {
    return ((code.onesCount + code.parityBit) % 2) == 1;
}

scan_code read_scancode(ps2_device_t *dev) {    
    scan_code scannedCode;
    int bit;

    while (true) {                   
        if (!read_bit(dev, &bit)) continue;
        if (dev->resync) continue;  
        if (bit != 0) continue;
        int startBit = bit; // Will always be 0

        int onesCount = 0;
        uint8_t data = 0;
        
        // DATA BITS
        for (int i = 0; i < 8; i++) {
            if (!read_bit(dev, &bit)) goto resync;
            
            data |= (bit & 1) << i;
            onesCount += (bit & 1);
        }

        // PARITY BIT
        if (!read_bit(dev, &bit)) goto resync;
        int parityBit = bit;

        // STOP BIT
        if (!read_bit(dev, &bit)) goto resync;
        int stopBit = bit;
        
        scannedCode.startBit = startBit; 
        scannedCode.dataNum = data;
        scannedCode.parityBit = parityBit;
        scannedCode.stopBit = stopBit;
        scannedCode.onesCount = onesCount;
        
        // Error handling for invalid parity and stop bits    
        if (!(is_parity_valid(scannedCode))) goto resync;
        if (!(scannedCode.stopBit == 1)) goto resync;
        
        return scannedCode;

    resync:
       dev->resync = true;   
       continue; 
    }
}

// Read a single PS2 scancode. Always returns a correctly received scancode:
// if an error occurs (e.g., start bit not detected, parity is wrong), the
// function should read another scancode.
// Read a single PS2 scan code.
uint8_t ps2_read(ps2_device_t *dev) {
    scan_code readCode = read_scancode(dev);
    return readCode.dataNum;
}

