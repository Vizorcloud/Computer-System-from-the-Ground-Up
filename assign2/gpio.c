/* File: gpio.
 * ------------
 * ***** TODO: add your file header comment here *****
 */
#include "gpio.h"
#include <stddef.h>

enum { GROUP_B = 0, GROUP_C, GROUP_D, GROUP_E, GROUP_F, GROUP_G };

typedef struct  {
    unsigned int group;
    unsigned int pinIndex;
    unsigned int bitShift;
    unsigned int configRegIndex;
} gpio_pin_t;

// The gpio_id_t enumeration assigns a symbolic constant for each
// in such a way to use a single hex constant. The more significant
// hex digit identifies the group and lower 2 hex digits are pin index:
//       constant 0xNnn  N = which group,  nn = pin index within group
//
// This helper function extracts the group, pin index, config register, and bitShift within its config register
// from the least significant bit from a gpio_id_t
// e.g. GPIO_PB4 belongs to GROUP_B, has pin_index 4, and has its config nibble 
static gpio_pin_t get_pin_info(gpio_id_t gpio) {
    gpio_pin_t gp;
    gp.group = gpio >> 8;
    gp.pinIndex = gpio & 0xff; // lower 2 hex digits
    gp.bitShift = (gp.pinIndex % 8) * 4;
    gp.configRegIndex = gp.pinIndex / 8;
    return gp;
}

// The gpio groups are differently sized, e.g. B has 13 pins, C only 8.
// This helper function confirms that a gpio_id_t is valid (group
// and pin index are valid)
bool gpio_id_is_valid(gpio_id_t pin) {
    gpio_pin_t gp = get_pin_info(pin);
    switch (gp.group) {
        case GROUP_B: return (gp.pinIndex <= GPIO_PB_LAST_INDEX);
        case GROUP_C: return (gp.pinIndex <= GPIO_PC_LAST_INDEX);
        case GROUP_D: return (gp.pinIndex <= GPIO_PD_LAST_INDEX);
        case GROUP_E: return (gp.pinIndex <= GPIO_PE_LAST_INDEX);
        case GROUP_F: return (gp.pinIndex <= GPIO_PF_LAST_INDEX);
        case GROUP_G: return (gp.pinIndex <= GPIO_PG_LAST_INDEX);
        default:      return false;
    }
}

// This helper function is suggested to return the address of
// the config0 register for a gpio group, i.e. get_cfg0_reg(GROUP_B)
// Refer to the D1-H user manual to learn the address the config0 register
// for each group. Be sure to note how the address of the config1 and
// config2 register can be computed as relative offset from config0.
// (okay to discard this function if it doesn't fit with your design)
static volatile unsigned int *get_cfg0_reg(unsigned int group) {
    /***** TODO: Your code goes here *****/
    return NULL;
}

// This helper function is suggested to return the address of
// the data register for a gpio group. Refer to the D1-H user manual
// to learn the address of the data register for each group.
// (okay to discard this function if it doesn't fit with your design)
static volatile unsigned int *get_data_reg(unsigned int group) {
    /***** TODO: Your code goes here *****/
    return NULL;
}

void gpio_init(void) {
    // no initialization required for this peripheral
}

void gpio_set_input(gpio_id_t pin) {
    gpio_set_function(pin, GPIO_FN_INPUT);
}

void gpio_set_output(gpio_id_t pin) {
    gpio_set_function(pin, GPIO_FN_OUTPUT);
}

volatile unsigned int *gpio_get_config_address(gpio_id_t pin) {
    unsigned int baseAddress = 0x02000000; // GPIO Module Base Address
    unsigned int configGroupSeparationVal = 0x30; // Space between each config group in memory

    gpio_pin_t gpioPin = get_pin_info(pin);

    volatile unsigned int *configRegAddress = (volatile unsigned int *)(baseAddress 
        + 0x30 + (configGroupSeparationVal * gpioPin.group) + gpioPin.configRegIndex);

    return configRegAddress;    
}

void gpio_set_function(gpio_id_t pin, unsigned int function) {
    if (!gpio_id_is_valid(pin)) return;
    if (function > GPIO_FN_DISABLED) return;    

    gpio_pin_t gpioPin = get_pin_info(pin);

    volatile unsigned int *configPtr = gpio_get_config_address(pin);
    volatile unsigned int configVal = *configPtr; 
    unsigned int mask = ~(0xF << gpioPin.bitShift);
    configVal &= mask;
    configVal |= function << gpioPin.bitShift;

    *configPtr = configVal;
}

unsigned int gpio_get_function(gpio_id_t pin) {
    if (!gpio_id_is_valid(pin)) return GPIO_INVALID_REQUEST;
    
    gpio_pin_t gpioPin = get_pin_info(pin);
    
    volatile unsigned int *configPtr = gpio_get_config_address(pin);
    volatile unsigned int configVal = *configPtr;
    unsigned int pinVal = (configVal >> gpioPin.bitShift) & 0xF;

    return pinVal;
}

void gpio_write(gpio_id_t pin, int value) {
    /***** TODO: Your code goes here *****/
}

int gpio_read(gpio_id_t pin) {
    /***** TODO: Your code goes here *****/
    return 0;
}

/*
void gpio_set_pullup(gpio_id_t pin) {
    ***** EXTENSION TODO: Your code goes here *****
    (If you are not doing the extension, leave this function unimplemented and commented out
}
*/
