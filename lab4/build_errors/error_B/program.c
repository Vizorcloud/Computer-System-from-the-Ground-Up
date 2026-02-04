#include "gpio.h"
#include "printf.h"

void main(void) {
    uart_init();

    uart_putstring("Hello, hola, ciao, tere\n");
    uart_putstring();           // how is okay that all of these very different calls
    uart_putstring(1, 4, 6);    // to same function uart_putstring are allowed?
    uart_putstring(main);       // if you run the compiled program, what happens?
    uart_putstring("BYE!");
}
