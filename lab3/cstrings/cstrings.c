#include "assert.h"
#include <stddef.h>
#include "uart.h"

// --------  functions to implement -------------------

static size_t strlen(const char *str) {
    int n = 0;
    while (str[n] != '\0') {
        n++;
    }
    return n;
}

static char *strcpy(char *dst, const char *src) {
    // TODO: Your turn -- implement strcpy!
    return dst;
}


// --------  unit tests from here down -------------------

void test_strlen(void) {
    char *fruit = "watermelon";

    assert(strlen("green") == 5);
    assert(strlen("") ==  0);
    assert(strlen(fruit) == 2 + strlen(fruit + 2));
}

void test_strcpy(const char *orig) {
    int len = strlen(orig);
    char buf[len + 1]; // plus one for terminator

    char *copy = strcpy(buf, orig);
    assert(copy != orig);
    for (int i = 0; i <= len; i++) // compare letter by letter
        assert(copy[i] == orig[i]);
}

void bogus_strlen_uninitialized(void) {
    char uninitialized[10];
    // what "should" happen if you call strlen on address of unintialized memory?
    // what *does* happen?
    assert(strlen(uninitialized) == 0);

}

void bogus_strlen_no_terminator(void) {
    int neighbor[2] = {0x7e7e7e7e, 0x7e7e7e7e};
    char no_terminator[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    // what "should" happen if you call strlen on a string with no terminator?
    // what *does* happen?
    assert(strlen(no_terminator) == 8);

}

void bogus_strlen_null_ptr(void) {
    // what "should" happen if you call strlen on a NULL pointer?
    // what *does* happen?
    assert(strlen(NULL) == 0);
}

void main(void) {
    uart_init();

    test_strlen();
    //test_strcpy("CS107e rocks"); // uncomment this test after you have implemented strcpy

    // below are tests that make wrong-headed call to strlen
    // uncomment these one by one and run to see what the consequences
    // of these calls
    // Try both using "make test" to run the test the actual Pi and
    // using "make debug" to run the test under the gdb simulator
    // Do you get same result?

    // bogus_strlen_uninitialized();   // bogus #1
    // bogus_strlen_no_terminator();   // bogus #2
    // bogus_strlen_null_ptr();        // bogus #3
}
