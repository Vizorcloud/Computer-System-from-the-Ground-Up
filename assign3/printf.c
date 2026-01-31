/* File: printf.c
 * --------------
 * ***** TODO: add your file header comment here *****
 */
#include "printf.h"
#include <stdarg.h>
#include <stdint.h>
#include "strings.h"
#include "uart.h"

/* Prototypes for internal helpers.
 * Typically we would qualify these functions as static (private to module)
 * but in order to call them from the test program, must declare externally
 */
void num_to_string(unsigned long num, int base, char *outstr);
const char *hex_string(unsigned long val);
const char *decimal_string(long val);

// max number of digits in long + space for negative sign and null-terminator
#define MAX_DIGITS 25


/* Convenience functions `hex_string` and `decimal_string` are provided
 * to you.  You should use the functions as-is, do not change the code!
 *
 * A key implementation detail to note is these functions declare
 * a buffer to hold the output string and return the address of buffer
 * to the caller. If that buffer memory were located on stack, it would be
 * incorrect to use pointer after function exit because local variables
 * are deallocated. To ensure the buffer memory is accessible after
 * the function exists, the declaration is qualified `static`. Memory
 * for a static variable is not stored on stack, but instead in the global data
 * section, which exists outside of any function call. Additionally static
 * makes it so there is a single copy of the variable, which is shared by all
 * calls to the function. Each time you call the function, it overwrites/reuses
 * the same variable/memory.
 *
 * Adding static qualifier to a variable declared inside a function is a
 * highly atypical practice and appropriate only in very specific situations.
 * You will likely never need to do this yourself.
 * Come talk to us if you want to know more!
 */

const char *hex_string(unsigned long val) {
    // static buffer to allow use after function returns (see note above)
    static char buf[MAX_DIGITS];
    num_to_string(val, 16, buf); // num_to_string does the hard work
    return buf;
}

const char *decimal_string(long val) {
    // static buffer to allow use after function returns (see note above)
    static char buf[MAX_DIGITS];
    if (val < 0) {
        buf[0] = '-';   // add negative sign in front first
        num_to_string(-val, 10, buf + 1); // pass positive val as arg, start writing at buf + 1
    } else {
        num_to_string(val, 10, buf);
    }
    return buf;
}

void num_to_string(unsigned long num, int base, char *outstr) {
    /***** TODO: Your code goes here *****/
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args) {
    /***** TODO: Your code goes here *****/
    return 0;
}

int snprintf(char *buf, size_t bufsize, const char *format, ...) {
    /***** TODO: Your code goes here *****/
    return 0;
}

// ok to assume printf output is never longer that MAX_OUTPUT_LEN
#define MAX_OUTPUT_LEN 1024

int printf(const char *format, ...) {
    /***** TODO: Your code goes here *****/
    return 0;
}


/* From here to end of file is some sample code and suggested approach
 * for those of you doing the disassemble extension. Otherwise, ignore!
 *
 * The struct insn bitfield is declared using exact same layout as bits are organized in
 * the encoded instruction. Accessing struct.field will extract just the bits
 * apportioned to that field. If you look at the assembly the compiler generates
 * to access a bitfield, you will see it simply masks/shifts for you. Neat!
 */
/*
static const char *reg_names[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                                    "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                                    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                                    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };

struct insn  {
    uint32_t opcode: 7;
    uint32_t reg_d:  5;
    uint32_t funct3: 3;
    uint32_t reg_s1: 5;
    uint32_t reg_s2: 5;
    uint32_t funct7: 7;
};

void sample_use(unsigned int *addr) {
    struct insn in = *(struct insn *)addr;
    printf("opcode is 0x%x, reg_dst is %s\n", in.opcode, reg_names[in.reg_d]);
}
*/
