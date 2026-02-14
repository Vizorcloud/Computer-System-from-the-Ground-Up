#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"
#include "uart.h"

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
struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
};

// Creates a new PS2 device connected to given clock and data pins,
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
    return dev;
}

int read_bit(ps2_device_t *dev) {
    // Ensure we start HIGH before reading
    while (gpio_read(dev->clock) == 0) {
        ;
    }
    // Wait until the clock pin reads LOW
    while (gpio_read(dev->clock) == 1) {
        ;
    }

    // Read a bit from the data line
    int dataState = gpio_read(dev->data); 

    return dataState;
}

uint8_t bits_to_uint(int array[], int size) {
    uint8_t result = 0;

    for (int i = 0; i < size; i++) {
        result |= (array[i] & 1) << i;
    }

    return result;
}

// Read a single PS2 scan code.
uint8_t ps2_read(ps2_device_t *dev) { 
    // Read the start bit until its 1
    int startBit = read_bit(dev);
    
    while (startBit != 0) {
        startBit = read_bit(dev);
    }
    
    // Read and store the first 8 bits after the start bit
    int dataBuffer[8];
    int curBit;

    for (int i = 0; i < 8; i++) {
        curBit = read_bit(dev);
        dataBuffer[i] = curBit;    
    }

    int parityBit = read_bit(dev);
    int stopBit = read_bit(dev);
    
    // Decypher data bits into number
    return bits_to_uint(dataBuffer, 8);
}
