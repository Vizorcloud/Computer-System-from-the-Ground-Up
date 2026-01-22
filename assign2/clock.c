/* File: clock.c
 * -------------
 * ***** TODO: add your file header comment here *****
 */
#include "gpio.h"
#include "timer.h"

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

int main(void) {
    int countdown = DURATION;
    /***** TODO: Your code goes here *****/
    return countdown;
}
