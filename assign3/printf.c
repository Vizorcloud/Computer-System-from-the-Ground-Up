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

unsigned char toChar(unsigned int num) {
    if (num < 10) {
        return '0' + num;
    } else {
        return 'a' + (num - 10);
    }
}

void num_to_string(unsigned long num, int base, char *outstr) {
    unsigned int buffer[64];
    unsigned long dividend = num;
    unsigned int remainder = 0;
    unsigned int index = 0;

    // Handle special case
    if (num == 0) {
        outstr[0] = '0';
        outstr[1] = '\0';
        return;
    }

    if (base == 10) {
        while (dividend > 0) {
            remainder = dividend % 10;
            buffer[index] = remainder; 
            dividend = dividend / 10;
            index++;
        }
    } else if (base == 16) { 
        while (dividend > 0) {
            remainder = dividend % 16;
            buffer[index] = remainder; 
            dividend = dividend / 16;
            index++;
        }
    }
    
    int indexOffset = index - 1;

    for (int i = indexOffset; i >= 0; i--) {
        outstr[indexOffset - i] = toChar(buffer[i]);
    }

    outstr[index] = '\0';
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args) {
    /***** TODO: Your code goes here *****/
    return 0;
}

int snprintf(char *buf, size_t bufsize, const char *format, ...) {
    size_t bufIndex = 0;
    size_t formatIndex = 0;
    
    va_list ap;
    va_start(ap, format); // init va_list, read arguments following argument named format
    
    while (format[formatIndex] != '\0') {
        if (format[formatIndex] == '%') {
            formatIndex++;
            
            unsigned long fieldWidth = 0;
                        
            if (format[formatIndex] >= '0' && format[formatIndex] <= '9') {
                const char *afterWidth;
                fieldWidth = strtonum(&format[formatIndex], &afterWidth);
                formatIndex = afterWidth - format;                       
            }
            
            switch (format[formatIndex]) {
                case 'l': {
                    formatIndex++;

                    switch (format[formatIndex]) {
                        case 'd': {
                            long decimal = va_arg(ap, long);
                            const char *decimalStr = decimal_string(decimal);
                            int decimalIndex = 0;
                            int numLen = strlen(decimalStr);
                            
                            int padCount = fieldWidth > numLen ? fieldWidth - numLen : 0;
                            char padChar = ' ';  // Space for decimal
                            
                            for (int i = 0; i < padCount; i++) {
                                if (bufIndex < bufsize - 1) {
                                    buf[bufIndex] = padChar;
                                }
                                bufIndex++;
                            }

                            while (decimalStr[decimalIndex] != '\0') {
                                if (bufIndex < bufsize - 1) {
                                    buf[bufIndex] = decimalStr[decimalIndex];
                                }
                                decimalIndex++;
                                bufIndex++;
                            }

                            break;
                        }
                        case 'x': {
                            unsigned long hex = va_arg(ap, unsigned long);
                            const char *hexStr = hex_string(hex);
                            int hexLen = strlen(hexStr);
    
                            int padCount = fieldWidth > hexLen ? fieldWidth - hexLen : 0;
                            char padChar = '0';  // Zero for hex
    
                            for (int i = 0; i < padCount; i++) {
                                if (bufIndex < bufsize - 1) {
                                    buf[bufIndex] = padChar;
                                }
                                bufIndex++;
                            }
                            
                            int hexIndex = 0;
                            
                            while (hexStr[hexIndex] != '\0') {
                                if (bufIndex < bufsize - 1) {
                                    buf[bufIndex] = hexStr[hexIndex];
                                }
                                bufIndex++;
                                hexIndex++;
                            }
                            
                            break;
                        }
                    }
                    break;  
                }
                case 'd': {
                    int decimal = va_arg(ap, int);
                    const char *decimalStr = decimal_string(decimal);
                    int decimalIndex = 0;
                                        
                    int numLen = strlen(decimalStr);
                            
                    int padCount = fieldWidth > numLen ? fieldWidth - numLen : 0;
                    char padChar = ' ';  // Space for decimal
                            
                    for (int i = 0; i < padCount; i++) {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = padChar;
                        }
                        bufIndex++;
                    }

                    while (decimalStr[decimalIndex] != '\0') {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = decimalStr[decimalIndex];
                        }
                        decimalIndex++;
                        bufIndex++;
                    }
                    break;
                }
                case 's': {
                    char *str = va_arg(ap, char *);
                    int strIndex = 0;
                    int strLen = strlen(str);

                    int padCount = fieldWidth > strLen ? fieldWidth - strLen : 0;
                    char padChar = ' ';

                    for (int i = 0; i < padCount; i++) {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = padChar; // strings always space-padded    
                        }
                        bufIndex++;
                    }

                    while (str[strIndex] != '\0') {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = str[strIndex];
                        }
                        bufIndex++;
                        strIndex++;
                    }

                    break;
                }
                case 'c': {
                    int ch = va_arg(ap, int);
                    
                    int padCount = fieldWidth > 1 ? fieldWidth - 1 : 0;                    
                    char padChar = ' ';

                    for (int i = 0; i < padCount; i++) {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = padChar;
                        }
                        bufIndex++;
                    }

                    if (bufIndex < bufsize - 1) {
                        buf[bufIndex] = ch;
                    }
                    bufIndex++;
                    break;
                }
                case 'x': {
                    unsigned long hex = va_arg(ap, unsigned long);
                    const char *hexStr = hex_string(hex);
                                    
                    int hexLen = strlen(hexStr);
    
                    int padCount = fieldWidth > hexLen ? fieldWidth - hexLen : 0;
                    char padChar = '0';  // Zero for hex
    
                    for (int i = 0; i < padCount; i++) {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = padChar;
                        }
                        bufIndex++;
                    }
                    
                    int hexIndex = 0;
                    
                    while (hexStr[hexIndex] != '\0') {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = hexStr[hexIndex];
                        }
                        bufIndex++;
                        hexIndex++;
                    }
                    break;
                }
                case 'p': {
                    unsigned long ptr = (unsigned long)va_arg(ap, void *);
                    const char *hexStr = hex_string(ptr); // use existing hex_string
                    int hexLen = strlen(hexStr);
                    int totalLen = hexLen + 2;
                    int padCount = fieldWidth > totalLen ? fieldWidth - totalLen : 0;

                    for (int i = 0; i < padCount; i++) {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = '0';
                        }
                        bufIndex++;
                    }

                    if (bufIndex < bufsize - 1) buf[bufIndex] = '0';
                    bufIndex++;
                    if (bufIndex < bufsize - 1) buf[bufIndex] = 'x';
                    bufIndex++;

                    for (int i = 0; i < hexLen; i++) {
                        if (bufIndex < bufsize - 1) {
                            buf[bufIndex] = hexStr[i];
                        }
                        bufIndex++;
                    }
                    break;
                }
                case '%': {
                    if (bufIndex < bufsize - 1) {
                        buf[bufIndex] = '%';
                    }
                    bufIndex++;
                    break;
                }
                default:
                    break;
            }
            formatIndex++;
        } else {
            if (bufIndex < bufsize - 1) {
                buf[bufIndex] = format[formatIndex];
            }
            formatIndex++;
            bufIndex++;
        }
    }

    if (bufsize > 0) {
        buf[bufIndex < bufsize ? bufIndex : bufsize - 1] = '\0';
    }    
    va_end(ap);

    return bufIndex;
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
