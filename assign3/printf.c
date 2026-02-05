/* File: printf.c
 * --------------
 * Purpose: Printf Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 5 2025
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

static const char *reg_names[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                                    "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                                    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                                    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6" };

const char* disassemble_rtype(uint32_t instr) {
    static char buf[64];  // static so it persists after return
    uint8_t rd = (instr >> 7) & 0x1F;
    uint8_t funct3 = (instr >> 12) & 0x7;
    uint8_t rs1 = (instr >> 15) & 0x1F;
    uint8_t rs2 = (instr >> 20) & 0x1F;
    uint8_t funct7 = (instr >> 25) & 0x7F;

    const char *mnemonic = NULL;

    switch(funct3) {
        case 0x0: mnemonic = (funct7==0x00) ? "add" : "sub"; break;
        case 0x1: mnemonic = "sll"; break;
        case 0x2: mnemonic = "slt"; break;
        case 0x3: mnemonic = "sltu"; break;
        case 0x4: mnemonic = "xor"; break;
        case 0x5: mnemonic = (funct7==0x00) ? "srl" : "sra"; break;
        case 0x6: mnemonic = "or"; break;
        case 0x7: mnemonic = "and"; break;
    }

    if (!mnemonic) {
        snprintf(buf, sizeof(buf), ".word 0x%08x", instr);
    } else {
        snprintf(buf, sizeof(buf), "%s %s,%s,%s",
            mnemonic,
            reg_names[rd],
            reg_names[rs1],
            reg_names[rs2]);
    }
    return buf;
}

const char* disassemble_itype(uint32_t instr) {
    static char buf[64];
    uint8_t rd     = (instr >> 7) & 0x1F;
    uint8_t funct3 = (instr >> 12) & 0x7;
    uint8_t rs1    = (instr >> 15) & 0x1F;
    int32_t imm = (instr >> 20) & 0xFFF;  // 12-bit
    if (imm & 0x800) imm |= 0xFFFFF000;   // sign-extend

    const char *mnemonic = NULL;
    switch(funct3) {
        case 0x0: mnemonic = "addi"; break;
        case 0x2: mnemonic = "slti"; break;
        case 0x3: mnemonic = "sltiu"; break;
        case 0x4: mnemonic = "xori"; break;
        case 0x6: mnemonic = "ori"; break;
        case 0x7: mnemonic = "andi"; break;
        case 0x1: mnemonic = "slli"; imm &= 0x1F; break;
        case 0x5: mnemonic = ((instr >> 30) & 1) ? "srai" : "srli"; imm &= 0x1F; break;
        default: snprintf(buf, sizeof(buf), ".word 0x%08x", instr); return buf;
    }
    
    if (!mnemonic) {
        snprintf(buf, sizeof(buf), ".word 0x%08x", instr);
    } else {
        snprintf(buf, sizeof(buf), "%s %s,%s,%d", mnemonic, 
            reg_names[rd], 
            reg_names[rs1], 
            imm); // immediate value
    }
    return buf;
}

const char* disassemble_sbtype(uint32_t instr, unsigned long addr) {
    static char buf[64];
    uint8_t funct3 = (instr >> 12) & 0x7;
    uint8_t rs1    = (instr >> 15) & 0x1F;
    uint8_t rs2    = (instr >> 20) & 0x1F;

    int32_t imm = ((instr >> 7 & 0x1E) | ((instr >> 25) & 0x3F) << 5 |
                  ((instr >> 31) << 12) | ((instr >> 7) & 1) << 11);
    imm = (imm << 19) >> 19; // sign-extend

    const char *mnemonic = NULL;
    switch(funct3) {
        case 0x0: mnemonic = "beq"; break;
        case 0x1: mnemonic = "bne"; break;
        case 0x4: mnemonic = "blt"; break;
        case 0x5: mnemonic = "bge"; break;
        case 0x6: mnemonic = "bltu"; break;
        case 0x7: mnemonic = "bgeu"; break;
        default: snprintf(buf, sizeof(buf), ".word 0x%08x", instr); return buf;
    }
    
    if (!mnemonic) {
        snprintf(buf, sizeof(buf), ".word 0x%08x", instr);
    } else {
        snprintf(buf, sizeof(buf), "%s %s,%s,0x%lx",
            mnemonic,
            reg_names[rs1],
            reg_names[rs2],
            addr + imm); // branch target
    }
    return buf;
}

const char* disassemble_ujtype(uint32_t instr, unsigned long addr) {
    static char buf[64];
    uint8_t rd = (instr >> 7) & 0x1F;

    int32_t imm = ((instr >> 12 & 0xFF) << 12) |
                  ((instr >> 20 & 1) << 11) |
                  ((instr >> 21 & 0x3FF) << 1) |
                  ((instr >> 31) << 20);
    imm = (imm << 11) >> 11; // sign-extend 

    const char *mnemonic = "jal";  

    snprintf(buf, sizeof(buf), "%s %s,0x%lx", mnemonic, reg_names[rd], addr + imm);

    return buf;
}

const char* disassemble(uint32_t instr, unsigned long addr) {
    uint8_t opcode = instr & 0x7F;

    switch(opcode) {
        case 0x33:  // R-type
            return disassemble_rtype(instr); 
        case 0x13:  // I-type
            return disassemble_itype(instr); 
        case 0x63:  // SB-type
            return disassemble_sbtype(instr, addr);
        case 0x6F:  // UJ-type
            return disassemble_ujtype(instr, addr);
        default:
            static char buf[64];
            snprintf(buf, sizeof(buf), ".word 0x%08x", instr);
            return buf;
    }
}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list args) {           
    size_t bufIndex = 0;
    size_t formatIndex = 0;
     
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
                            long decimal = va_arg(args, long);
                            const char *decimalStr = decimal_string(decimal);
                            int decimalIndex = 0;
                            int numLen = strlen(decimalStr);
                            
                            int padCount = fieldWidth > numLen ? fieldWidth - numLen : 0;
                            char padChar = ' ';  
                            
                            for (int i = 0; i < padCount; i++) {
                                if (bufsize > 0 && bufIndex < bufsize - 1) {
                                    buf[bufIndex] = padChar;
                                }
                                bufIndex++;
                            }

                            while (decimalStr[decimalIndex] != '\0') {
                                if (bufsize > 0 && bufIndex < bufsize - 1) {
                                    buf[bufIndex] = decimalStr[decimalIndex];
                                }
                                decimalIndex++;
                                bufIndex++;
                            }

                            break;
                        }
                        case 'x': {
                            unsigned long hex = va_arg(args, unsigned long);
                            const char *hexStr = hex_string(hex);
                            int hexLen = strlen(hexStr);
    
                            int padCount = fieldWidth > hexLen ? fieldWidth - hexLen : 0;
                            char padChar = '0';  // Zero for hex
    
                            for (int i = 0; i < padCount; i++) {
                                if (bufsize > 0 && bufIndex < bufsize - 1) {
                                    buf[bufIndex] = padChar;
                                }
                                bufIndex++;
                            }
                            
                            int hexIndex = 0;
                            
                            while (hexStr[hexIndex] != '\0') {
                                if (bufsize > 0 && bufIndex < bufsize - 1) {
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
                    int decimal = va_arg(args, int);
                    const char *decimalStr = decimal_string(decimal);
                    int decimalIndex = 0;
                                        
                    int numLen = strlen(decimalStr);
                            
                    int padCount = fieldWidth > numLen ? fieldWidth - numLen : 0;
                    char padChar = ' ';  
                            
                    for (int i = 0; i < padCount; i++) {
                        if (bufsize > 0 && bufIndex < bufsize - 1) {
                            buf[bufIndex] = padChar;
                        }
                        bufIndex++;
                    }

                    while (decimalStr[decimalIndex] != '\0') {
                        if (bufsize > 0 && bufIndex < bufsize - 1) {
                            buf[bufIndex] = decimalStr[decimalIndex];
                        }
                        decimalIndex++;
                        bufIndex++;
                    }
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char *);
                    int strIndex = 0;
                    int strLen = strlen(str);

                    int padCount = fieldWidth > strLen ? fieldWidth - strLen : 0;
                    char padChar = ' ';

                    for (int i = 0; i < padCount; i++) {
                        if (bufsize > 0 && bufIndex < bufsize - 1) {
                            buf[bufIndex] = padChar; // strings always space-padded    
                        }
                        bufIndex++;
                    }

                    while (str[strIndex] != '\0') {
                        if (bufsize > 0 && bufIndex < bufsize - 1) {
                            buf[bufIndex] = str[strIndex];
                        }
                        bufIndex++;
                        strIndex++;
                    }

                    break;
                }
                case 'c': {
                    int ch = va_arg(args, int);
                    
                    int padCount = fieldWidth > 1 ? fieldWidth - 1 : 0;                    
                    char padChar = ' ';

                    for (int i = 0; i < padCount; i++) {
                        if (bufsize > 0 && bufIndex < bufsize - 1) {
                            buf[bufIndex] = padChar;
                        }
                        bufIndex++;
                    }

                    if (bufsize > 0 && bufIndex < bufsize - 1) {
                        buf[bufIndex] = ch;
                    }
                    bufIndex++;
                    break;
                }
                case 'x': {
                    unsigned long hex = va_arg(args, unsigned long);
                    const char *hexStr = hex_string(hex);
                                    
                    int hexLen = strlen(hexStr);
    
                    int padCount = fieldWidth > hexLen ? fieldWidth - hexLen : 0;
                    char padChar = '0';  // Zero for hex
    
                    for (int i = 0; i < padCount; i++) {
                        if (bufsize > 0 && bufIndex < bufsize - 1) {
                            buf[bufIndex] = padChar;
                        }
                        bufIndex++;
                    }
                    
                    int hexIndex = 0;
                    
                    while (hexStr[hexIndex] != '\0') {
                        if (bufsize > 0 && bufIndex < bufsize - 1) {
                            buf[bufIndex] = hexStr[hexIndex];
                        }
                        bufIndex++;
                        hexIndex++;
                    }
                    break;
                }
                case 'p': {
                    formatIndex++;
                    
                    if (format[formatIndex] == 'I') {
                        // handle %pI
                        unsigned int *ptr = va_arg(args, unsigned int *);
                        uint32_t instr = *ptr;
                        unsigned long addr = (unsigned long)ptr;
                        const char *instrStr = disassemble(instr, addr);
                        // Print in the format "0x<addr>: <disassembly>"
                        int n = snprintf(buf + bufIndex, bufsize - bufIndex, "0x%016lx:\t%s", (unsigned long)ptr, instrStr);
                        bufIndex += n;
                    } else {
                        formatIndex--; // Backtrace

                        unsigned long ptr = (unsigned long)va_arg(args, void *);
                        const char *hexStr = hex_string(ptr);
                        int hexLen = strlen(hexStr);
                        int totalLen = hexLen + 2;
                        int padCount = fieldWidth > totalLen ? fieldWidth - totalLen : 0;

                        if (bufsize > 0 && bufIndex < bufsize - 1) buf[bufIndex] = '0';
                        bufIndex++;
                        if (bufsize > 0 && bufIndex < bufsize - 1) buf[bufIndex] = 'x';
                        bufIndex++;
                 
                        for (int i = 0; i < padCount; i++) {
                            if (bufsize > 0 && bufIndex < bufsize - 1) {
                                buf[bufIndex] = '0';
                            }
                            bufIndex++;
                        }

                        for (int i = 0; i < hexLen; i++) {
                            if (bufsize > 0 && bufIndex < bufsize - 1) {
                                buf[bufIndex] = hexStr[i];
                            }
                            bufIndex++;
                        }
                    }
                    break;
                }
                case '%': {
                    if (bufsize > 0 && bufIndex < bufsize - 1) {
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
            if (bufsize > 0 && bufIndex < bufsize - 1) {
                buf[bufIndex] = format[formatIndex];
            }
            formatIndex++;
            bufIndex++;
        }
    }

    if (bufsize > 0) {
        buf[bufIndex < bufsize ? bufIndex : bufsize - 1] = '\0';
    }    

    return bufIndex;
}

int snprintf(char *buf, size_t bufsize, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int n = vsnprintf(buf, bufsize, format, ap);
    va_end(ap);
    return n;
}

// ok to assume printf output is never longer that MAX_OUTPUT_LEN
#define MAX_OUTPUT_LEN 1024

int printf(const char *format, ...) {
    size_t bufsize = 1024;
    char buf[bufsize];
    va_list ap;
    va_start(ap, format);
    int n = vsnprintf(buf, bufsize, format, ap);
    va_end(ap);

    uart_putstring(buf);
    return n;
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
struct insn {
    uint32_t opcode: 7;
    uint32_t rd: 5;
    uint32_t funct3: 3;
    uint32_t rs1: 5;
    uint32_t rs2: 5;
    uint32_t funct7: 7;
};

void sample_use(unsigned int *addr) {
    struct insn in = *(struct insn *)addr;
    printf("opcode is 0x%x, reg_dst is %s\n", in.opcode, reg_names[in.reg_d]);
}
*/
