/* File: test_strings_printf.c
 * ---------------------------
 *  Purpose: String/Printf Module Library Implementation Testing
 *  Name: Maxsem Garcia
 *  Course: CS107E Tuesday Lab
 *  Date Last Modified: Feb 5 2025
 */
#include "assert.h"
#include "printf.h"
#include <stddef.h>
#include "strings.h"
#include "uart.h"

// Prototypes copied from printf.c to allow unit testing of helper functions
void num_to_string(unsigned long num, int base, char *outstr);
const char *hex_string(unsigned long val);
const char *decimal_string(long val);


static void test_memset(void) {
    char buf[25];
    size_t bufsize = sizeof(buf);

    memset(buf, 0x7e, bufsize);
    for (int i = 0; i < bufsize; i++)
        assert(buf[i] == 0x7e);

    int i;

    // Fill with zero
    memset(buf, 0, bufsize);
    for (i = 0; i < bufsize; i++) {
        assert(buf[i] == 0);
    }

    // Fill with negative value (treated as unsigned char)
    memset(buf, -1, bufsize);
    for (i = 0; i < bufsize; i++) {
        assert((unsigned char)buf[i] == 0xFF);
    }

    // Fill only part of the buffer
    memset(buf, 0x12, 10);
    for (i = 0; i < 10; i++) {
        assert((unsigned char)buf[i] == 0x12);
    }
    for (i = 10; i < bufsize; i++) {
        assert((unsigned char)buf[i] == 0xFF); // leftover from previous test
    }

    // Fill zero bytes — use fresh buffer
    memset(buf, 0xFF, bufsize);   // re-initialize
    memset(buf, 0x34, 0);         // zero-length fill
    for (i = 0; i < bufsize; i++) {
        assert((unsigned char)buf[i] == 0xFF); // unchanged
    }

    // Fill the buffer again with 0x00
    memset(buf, 0x00, bufsize);
    for (i = 0; i < bufsize; i++) {
        assert(buf[i] == 0x00);
    }
}

static void test_strcmp(void) {
    assert(strcmp("me", "me") == 0);
    assert(strcmp("me", "you") != 0);

    assert(strcmp("", "") == 0);
    assert(strcmp("me", "") > 0);
    assert(strcmp("", "me") < 0);
    
    assert(strcmp("me", "me") == 0);
    assert(strcmp("me", "me") == 0);
    assert(strcmp("me", "you") != 0);

    assert(strcmp("abc", "abcd") < 0);
    assert(strcmp("abcd", "abc") > 0);

    assert(strcmp("a", "A") > 0);
    assert(strcmp("A", "a") < 0);

    assert(strcmp("", "") == 0);
    assert(strcmp("", "abc") < 0);
    assert(strcmp("abc", "") > 0);

    assert(strcmp("abc", "bbc") < 0);
    assert(strcmp("bbc", "abc") > 0);

    assert(strcmp("abcd", "abce") < 0);
    assert(strcmp("abce", "abcd") > 0);

    assert(strcmp("abc1", "abc2") < 0);
    assert(strcmp("abc#", "abc$") < 0);
}

static void test_strlcat(void) {
    char buf[20];
    size_t bufsize = sizeof(buf);
    // as aid for debugging, fill contents of buffer with repeat value 0x7e
    // rather than leave contents uninitialized, this makes it easier to
    // spot later if contents have not changed in the way you intend
    memset(buf, 0x7e, bufsize); // for debug, init array contents with fixed repeat value

    buf[0] = '\0'; // null at first index makes empty string
    assert(strcmp(buf, "") == 0);
    strlcat(buf, "CS", bufsize); // append CS
    assert(strcmp(buf, "CS") == 0);
    strlcat(buf, "107e", bufsize); // append 107e
    assert(strcmp(buf, "CS107e") == 0);

    size_t r;

    // Simple append fits
    memset(buf, 0x7e, bufsize);
    buf[0] = 'H';
    buf[1] = 'i';
    buf[2] = '\0';
    r = strlcat(buf, "!", bufsize);
    assert(strcmp(buf, "Hi!") == 0);
    assert(r == 3); // "Hi" + "!"

    // Append multiple characters
    memset(buf, 0x7e, bufsize);
    buf[0] = 'C';
    buf[1] = 'S';
    buf[2] = '\0';
    r = strlcat(buf, "107e", bufsize);
    assert(strcmp(buf, "CS107e") == 0);
    assert(r == 6); // "CS" + "107e"

    // Append with truncation (dstsize smaller than needed)
    memset(buf, 0x7e, bufsize);
    buf[0] = 'H';
    buf[1] = 'e';
    buf[2] = 'l';
    buf[3] = 'l';
    buf[4] = 'o';
    buf[5] = '\0';
    r = strlcat(buf, "WorldThisIsTooLong", 10);
    assert(strcmp(buf, "HelloWorl") == 0); // fits 9 chars + '\0'
    assert(r == 5 + strlen("WorldThisIsTooLong"));

    // No space to append (dstsize == strlen(dst) + 1)
    memset(buf, 0x7e, bufsize);
    buf[0] = 'A';
    buf[1] = 'B';
    buf[2] = 'C';
    buf[3] = 'D';
    buf[4] = 'E';
    buf[5] = '\0';
    r = strlcat(buf, "FG", 6);
    assert(strcmp(buf, "ABCDE") == 0); // unchanged
    assert(r == 5 + 2); // total length if there was space

    // dstsize smaller than strlen(dst) (nothing copied)
    memset(buf, 0x7e, bufsize);
    buf[0] = 'A';
    buf[1] = 'B';
    buf[2] = 'C';
    buf[3] = 'D';
    buf[4] = 'E';
    buf[5] = 'F';
    buf[6] = 'G';
    buf[7] = '\0';
    r = strlcat(buf, "HIJ", 4);
    assert(strcmp(buf, "ABCDEFG") == 0); // unchanged
    assert(r == 4 + strlen("HIJ")); // 4 = dstsize, spec behavior

    // Empty src
    memset(buf, 0x7e, bufsize);
    buf[0] = 'T';
    buf[1] = 'e';
    buf[2] = 's';
    buf[3] = 't';
    buf[4] = '\0';
    r = strlcat(buf, "", bufsize);
    assert(strcmp(buf, "Test") == 0);
    assert(r == 4);

    // Empty dst
    memset(buf, 0x7e, bufsize);
    buf[0] = '\0';
    r = strlcat(buf, "ABC", bufsize);
    assert(strcmp(buf, "ABC") == 0);
    assert(r == 3);

    // dstsize = 0 (nothing copied, return length of src)
    memset(buf, 0x7e, bufsize);
    r = strlcat(buf, "ABC", 0);
    assert(r == 3);

}

static void test_strtonum(void) {
    assert(strtonum("67", NULL) == 67);

    const char *input = "107rocks";
    const char *rest = NULL;
    long val = strtonum(input, &rest);
    assert(val == 107);
    assert(rest == &input[3]);  // rest should now point to first non-digit character
    
    // Decimal numbers
    assert(strtonum("0", NULL) == 0);
    assert(strtonum("12345", NULL) == 12345);
    assert(strtonum("9abc", &rest) == 9 && rest == &"9abc"[1]);

    // Hexadecimal numbers
    assert(strtonum("0x0", NULL) == 0);
    assert(strtonum("0x1A", NULL) == 26);
    assert(strtonum("0x1fFz", &rest) == 511 && rest == &"0x1fFz"[5]);
    assert(strtonum("0XABC", NULL) == 0); // uppercase X and letters

    // Edge cases
    assert(strtonum("", NULL) == 0);          // empty string
    assert(strtonum("x123", NULL) == 0);     // invalid first character
    assert(strtonum("0x", NULL) == 0);       // hex prefix only, no digits

    // Large numbers
    assert(strtonum("4294967295", NULL) == 4294967295UL); // max 32-bit unsigned
    assert(strtonum("0xFFFFFFFF", NULL) == 0xFFFFFFFFUL); // max 32-bit hex

    // Mixing digits and letters
    assert(strtonum("12abc34", &rest) == 12 && rest == &"12abc34"[2]);
    assert(strtonum("0x12G34", &rest) == 18 && rest == &"0x12G34"[4]);

    // endptr = NULL
    assert(strtonum("456xyz", NULL) == 456);
}


static void test_helpers(void) {
    char buf[32];
    size_t bufsize = sizeof(buf);
    memset(buf, 0x7e, bufsize); // for debug, init array contents with fixed repeat value

    num_to_string(45, 10, buf);
    assert(strcmp(buf, "45") == 0);
    num_to_string(45, 16, buf);
    assert(strcmp(buf, "2d") == 0);

    assert(strcmp(decimal_string(-88), "-88") == 0);
    assert(strcmp(hex_string(0x107e), "107e") == 0);

    // num_to_string: edge cases
    num_to_string(0, 10, buf);
    assert(strcmp(buf, "0") == 0);
    
    num_to_string(0, 16, buf);
    assert(strcmp(buf, "0") == 0);
    
    // num_to_string: single digits
    num_to_string(7, 10, buf);
    assert(strcmp(buf, "7") == 0);
    
    num_to_string(9, 16, buf);
    assert(strcmp(buf, "9") == 0);
    
    num_to_string(15, 16, buf);
    assert(strcmp(buf, "f") == 0);
    
    // num_to_string: larger numbers
    num_to_string(12345, 10, buf);
    assert(strcmp(buf, "12345") == 0);
    
    num_to_string(255, 16, buf);
    assert(strcmp(buf, "ff") == 0);
     
    num_to_string(200, 16, buf);
    assert(strcmp(buf, "c8") == 0);
    
    // num_to_string: powers of base
    num_to_string(10, 10, buf);
    assert(strcmp(buf, "10") == 0);
    
    num_to_string(100, 10, buf);
    assert(strcmp(buf, "100") == 0);
    
    num_to_string(16, 16, buf);
    assert(strcmp(buf, "10") == 0);
    
    num_to_string(256, 16, buf);
    assert(strcmp(buf, "100") == 0);
    
    // decimal_string: zero
    assert(strcmp(decimal_string(0), "0") == 0);
    
    // decimal_string: positive numbers
    assert(strcmp(decimal_string(1), "1") == 0);
    assert(strcmp(decimal_string(42), "42") == 0);
    assert(strcmp(decimal_string(9999), "9999") == 0);
    
    // decimal_string: negative numbers
    assert(strcmp(decimal_string(-1), "-1") == 0);
    assert(strcmp(decimal_string(-42), "-42") == 0);
    assert(strcmp(decimal_string(-9999), "-9999") == 0);
    
    // hex_string: zero
    assert(strcmp(hex_string(0), "0") == 0);
    
    // hex_string: small values
    assert(strcmp(hex_string(1), "1") == 0);
    assert(strcmp(hex_string(10), "a") == 0);
    assert(strcmp(hex_string(15), "f") == 0);
    
    // hex_string: larger values
    assert(strcmp(hex_string(0xff), "ff") == 0);
    assert(strcmp(hex_string(0x1234), "1234") == 0);
    assert(strcmp(hex_string(0xabcdef), "abcdef") == 0);
    assert(strcmp(hex_string(0xffffffff), "ffffffff") == 0);
}

static void test_snprintf(void) {
    char buf[100];
    size_t bufsize = sizeof(buf);
    memset(buf, 0x7e, bufsize); // for debug, init array contents with fixed repeat value
    
    //int ret_val = snprintf(buf, 3, "%p", (void *)0x107e);
    //printf("%d\n", ret_val);

    //printf("%p\n", (void *)0x107e);
    //snprintf(buf, 20, "%x", 0xffffffff); 
    //printf("%x\n", 0xffffffff);
    //assert(strcmp(buf, "ffffffff") == 0);

    //snprintf(buf, 10, "%x", 0x9abcdef0);
    //printf("%x\n", 0x9abcdef0);
    //assert(strcmp(buf, "9abcdef0") == 0);
    
    // No formatting codes
    snprintf(buf, bufsize, "Hello, world!");
    assert(strcmp(buf, "Hello, world!") == 0);

    // One string formatting code
    snprintf(buf, bufsize, "%s", "binky");
    assert(strcmp(buf, "binky") == 0);
    
    // Single character
    snprintf(buf, bufsize, "x");
    assert(strcmp(buf, "x") == 0);
    
    // String with spaces
    snprintf(buf, bufsize, "hello world");
    assert(strcmp(buf, "hello world") == 0);
    
    // String with special characters
    snprintf(buf, bufsize, "a!b@c#d$e");
    assert(strcmp(buf, "a!b@c#d$e") == 0);
    
    // Long string
    snprintf(buf, bufsize, "The quick brown fox jumps over the lazy dog");
    assert(strcmp(buf, "The quick brown fox jumps over the lazy dog") == 0);
    
    // Truncation test - small buffer
    char small[5];
    int n = snprintf(small, 5, "hello world");
    assert(strcmp(small, "hell") == 0);  // Should truncate to fit
    assert(n == 11);  // Should return full length that would be written
    
    // Truncation test - exact fit
    char exact[6];
    n = snprintf(exact, 6, "hello");
    assert(strcmp(exact, "hello") == 0);
    assert(n == 5);
    
    // Truncation test - one char buffer
    char tiny[1];
    n = snprintf(tiny, 1, "hello");
    assert(strcmp(tiny, "") == 0);  // Only null terminator fits
    assert(n == 5);
    
    // bufsize = 0 (should not crash)
    n = snprintf(NULL, 0, "hello");
    assert(n == 5);
    
    // Single char
    snprintf(buf, bufsize, "%c", 'A');
    assert(strcmp(buf, "A") == 0);

    // Single char with field width
    snprintf(buf, bufsize, "%3c", 'M');
    assert(strcmp(buf, "  M") == 0);

    // Decimal int
    snprintf(buf, bufsize, "%d", 42);
    assert(strcmp(buf, "42") == 0);

    // Decimal int with field width
    snprintf(buf, bufsize, "%5d", 7);
    assert(strcmp(buf, "    7") == 0);

    // Long decimal
    snprintf(buf, bufsize, "%ld", 1234567890L);
    assert(strcmp(buf, "1234567890") == 0);

    // Long decimal with field width
    snprintf(buf, bufsize, "%10ld", 1234L);
    assert(strcmp(buf, "      1234") == 0);

    // Hexadecimal
    snprintf(buf, bufsize, "%x", 0x1a3f);
    assert(strcmp(buf, "1a3f") == 0);

    // Hexadecimal with zero-padding
    snprintf(buf, bufsize, "%8x", 0x2b);
    assert(strcmp(buf, "0000002b") == 0);

    // Long hexadecimal
    snprintf(buf, bufsize, "%lx", 0x123abcdeL);
    assert(strcmp(buf, "123abcde") == 0);

    // Pointer
    void *ptr = (void *)0x1234abcd;
    snprintf(buf, bufsize, "%p", ptr);
    assert(strcmp(buf, "0x1234abcd") == 0);

    // Pointer with field width
    snprintf(buf, bufsize, "%12p", ptr);
    assert(strcmp(buf, "0x00001234abcd") == 0);

    // Percent literal
    snprintf(buf, bufsize, "%%");
    assert(strcmp(buf, "%") == 0);

    // Mixed formatting
    snprintf(buf, bufsize, "Char: %c, String: %s, Int: %d, Hex: %x, Ptr: %p, Percent: %%", 'Z', "test", 123, 0xAB, ptr);
    assert(strcmp(buf, "Char: Z, String: test, Int: 123, Hex: ab, Ptr: 0x1234abcd, Percent: %") == 0);

    // Truncation: bufsize smaller than output
    snprintf(buf, 5, "%7d", -9999);
    assert(strcmp(buf, "  -9") == 0); // Only first 4 chars fit
}


// This function just here as code to disassemble for extension
int sum(int n) {
    int result = 6;
    for (int i = 0; i < n; i++) {
        result += i * 3;
    }
    return result + 729;
}

void test_disassemble(void) {
    const unsigned int add =  0x00f706b3;
    const unsigned int xori = 0x0015c593;
    const unsigned int bne =  0xfe061ce3;
    const unsigned int sd =   0x02113423;
    const unsigned int j =    0x0000006f;

    // formatting code %pI accesses the disassemble extension.
    // If extension not implemented, regular version of printf
    // will simply output pointer address followed by I
    // e.g.  "... disassembles to 0x07ffffd4I"
    printf("Encoded instruction %08x disassembles to %pI\n", add, &add);
    printf("Encoded instruction %08x disassembles to %pI\n", xori, &xori);
    printf("Encoded instruction %08x disassembles to %pI\n", bne, &bne);
    printf("Encoded instruction %08x disassembles to %pI\n", sd, &sd);
    printf("Encoded instruction %08x disassembles to %pI\n", j, &j);

    unsigned int *fn = (unsigned int *)sum; // disassemble instructions from sum function
    for (int i = 0; i < 10; i++) {
        printf("%p:  %08x  %pI\n", &fn[i], fn[i], &fn[i]);
    }
}

void main(void) {
    uart_init();
    uart_putstring("Start execute main() in test_strings_printf.c\n");

    test_memset();
    test_strcmp();
    test_strlcat();
    test_strtonum();
    test_helpers();
    test_snprintf();
    test_disassemble(); // uncomment if you implement extension

    printf("Hello, world!\n");
    void *ptr = (void *)0x1234abcd;

    printf("Char test: '%c'\n", 'A');
    printf("Char with width: '%3c'\n", 'M');
    
    // Strings
    printf("String test: '%s'\n", "hello");
    printf("String with width: '%12s'\n", "binky");

    // Decimal integers
    printf("Decimal test: %d\n", 42);
    printf("Decimal with width: '%5d'\n", 7);
    printf("Long decimal test: %ld\n", 1234567890L);
    printf("Long decimal with width: '%10ld'\n", 1234L);

    // Hexadecimal
    printf("Hex test: %x\n", 0x1a3f);
    printf("Hex with zero-padding: '%8x'\n", 0x2b);
    printf("Long hex test: %lx\n", 0x123abcdeL);

    // Pointer
    printf("Pointer test: %p\n", ptr);
    printf("Pointer with width: '%12p'\n", ptr);

    // Percent literal
    printf("Percent test: %%\n");

    // Mixed
    printf("Mixed: Char: %c, String: %s, Int: %d, Hex: %x, Ptr: %p, Percent: %%\n",
           'Z', "test", 123, 0xAB, ptr);
    
    unsigned int add_instr = 0x00f706b3;
    unsigned int *instrPtr = &add_instr;

    printf("Pointer: %p\n", instrPtr);

    printf("Disassembled: %pI\n", instrPtr);

    printf("Before: %d, Instruction: %pI, After: %d\n", 123, instrPtr, 456);

    unsigned int addi_instr = 0x00108093;  // addi ra, ra, 1
    unsigned int *iPtr = &addi_instr;

    printf("Pointer: %p\n", iPtr);
    printf("Disassembled: %pI\n", iPtr);
    printf("Before: %d, Instruction: %pI, After: %d\n", 42, iPtr, 99);
    
    unsigned int beq_instr = 0x00208663;  // encoding of BEQ x1,x2,8
    unsigned int *beqPtr = &beq_instr;

    printf("SB-type test:\n");
    printf("Disassembled: %pI\n", beqPtr);
    
    unsigned int jal_instr = 0x010000EF;  // encoding of JAL x1,16
    unsigned int *jalPtr = &jal_instr;

    printf("UJ-type test:\n");
    printf("Disassembled: %pI\n", jalPtr);

    uart_putstring("Successfully finished executing main() in test_strings_printf.c\n");
}
