/* File: test_gpio_timer.c
 * -----------------------
 * ***** TODO: add your file header comment here *****
 */
#include "gpio.h"
#include "timer.h"

// You call assert on an expression that you expect to be true. If expr
// instead evaluates to false, then assert calls abort, which stops
// your program and flashes onboard led.
#define assert(expr) if(!(expr)) abort()

// infinite loop that flashes onboard blue LED (GPIO PD18)
void abort(void) {
    volatile unsigned int *GPIO_CFG2 = (unsigned int *)0x02000098;
    volatile unsigned int *GPIO_DATA = (unsigned int *)0x020000a0;

    // Configure GPIO PD18 function to be output.
    *GPIO_CFG2 = (*GPIO_CFG2 & ~(0xf00)) | 0x100;
    while (1) { // infinite loop
        *GPIO_DATA ^= (1 << 18); // invert value
        for (volatile int delay = 0x100000; delay > 0; delay--) ; // wait
    }
}

// Test all valid GPIO pins and functions
void test_gpio_full_range_student(void) {
    for (gpio_id_t pin = GPIO_ID_FIRST; pin <= GPIO_ID_LAST; pin++) {
        if (!gpio_id_is_valid(pin)) continue; // skip invalid pins

        for (unsigned int function = GPIO_FN_INPUT; function <= GPIO_FN_DISABLED; function++) {
            gpio_set_function(pin, function);
            assert(gpio_get_function(pin) == function);
        }
    }
}

void test_gpio_set_get_function(void) {
    // Test get pin function (pin defaults to disabled)
    assert( gpio_get_function(GPIO_PC0) == GPIO_FN_DISABLED);

    // Set pin to output, confirm get returns what was set
    gpio_set_output(GPIO_PC0);
    assert( gpio_get_function(GPIO_PC0) == GPIO_FN_OUTPUT );

    // Set pin to input, confirm get returns what was set
    gpio_set_input(GPIO_PC0);
    assert( gpio_get_function(GPIO_PC0) == GPIO_FN_INPUT );
}

void test_gpio_read_write(void) {
    // set pin to output before gpio_write
    gpio_set_output(GPIO_PB4);

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PB4, 0);
    assert( gpio_read(GPIO_PB4) ==  0 );

   // gpio_write high, confirm gpio_read reads what was written
    gpio_write(GPIO_PB4, 1);
    assert( gpio_read(GPIO_PB4) ==  1 );

    // gpio_write low, confirm gpio_read reads what was written
    gpio_write(GPIO_PB4, 0);
    assert( gpio_read(GPIO_PB4) ==  0 );
}

void test_timer(void) {
    // Test timer tick count incrementing
    unsigned long start = timer_get_ticks();
    for( int i=0; i<10; i++ ) { /* Spin */ }
    unsigned long finish = timer_get_ticks();
    assert( finish > start );

    // Test timer delay
    int usecs = 100;
    start = timer_get_ticks();
    timer_delay_us(usecs);
    finish = timer_get_ticks();
    assert( finish >= start + usecs*TICKS_PER_USEC );
}

void test_breadboard_connections(void) {
    const int N_SEG = 7, N_DIG = 4;
    gpio_id_t segment[] = {GPIO_PD17, GPIO_PB6, GPIO_PB12, GPIO_PB11, GPIO_PB10, GPIO_PD11, GPIO_PD13};
    gpio_id_t digit[] = {GPIO_PB4, GPIO_PB3, GPIO_PB2, GPIO_PC0};
    gpio_id_t button = GPIO_PD12;

    for (int i = 0; i < N_SEG; i++) {   // configure segments & digits as output
        gpio_set_output(segment[i]);
    }
    for (int i = 0; i < N_DIG; i++) {
        gpio_set_output(digit[i]);
    }
    gpio_set_input(button);         // configure button

    while (1) { // loop forever
        for (int i = 0; i < N_DIG; i++) {   // iterate over digits
            gpio_write(digit[i], 1);        // turn on digit
            for (int j = 0; j < N_SEG; j++) {   // iterate over segments
                gpio_write(segment[j], 1);      // turn on segment
                timer_delay_ms(200);
                while (gpio_read(button) == 0)
                    ;                       // pause while button pressed
                gpio_write(segment[j], 0);  // turn off segment
            }
            gpio_write(digit[i], 0);    // turn off digit
        }
    }
}

void main(void) {
    gpio_init();
    timer_init();

    // Uncomment the call to each test function below when you have implemented
    // the functions and are ready to test them

    test_gpio_set_get_function();
    test_gpio_full_range_student();
    // test_gpio_read_write();
    // test_timer();
    // test_breadboard_connections();
}
