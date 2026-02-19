/* File: shell.c
 * -------------
 * Purpose: Shell Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 17 2025
 */
#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "strings.h"
#include <stdbool.h>
#include "mango.h"

#define LINE_LEN 80
#define MAX_HISTORY 10
#define MAX_ARGS 50
#define MAX_COMMAND_LENGTH 20

int cmd_history(int argc, const char **argv);  
int cmd_repeat(int argc, const char **argv); 
int cmd_disassemble(int argc, const char **argv); 

// Module-level global variables for shell
static struct {
    input_fn_t shell_read;
    formatted_fn_t shell_printf;
} module;

typedef struct {
    char history[MAX_HISTORY][LINE_LEN]; // Buffer
    int head; // Index of head
    int tail; // Index of Tail
    int count;   // number of valid entries
    int countOfAllEntries;
} circularHistory;

circularHistory commandHistory = { .head = -1, .tail = -1 };
char previousCommand[LINE_LEN] = "";

// NOTE TO STUDENTS:
// Your shell commands output various information and respond to user
// error with helpful messages. The specific wording and format of
// these messages would not generally be of great importance, but
// in order to streamline grading, we ask that you aim to match the
// output of the reference version.
//
// The behavior of the shell commands is documented in "shell_commands.h"
// https://cs107e.github.io/header#shell_commands
// The header file gives example output and error messages for all
// commands of the reference shell. Please match this wording and format.
//
// Your graders thank you in advance for taking this care!


static const command_t commands[] = {
    {"help",  "help [cmd]",  "print command usage and description", cmd_help},
    {"echo",  "echo [args]", "print arguments", cmd_echo},
    {"clear", "clear",       "clear screen (if your terminal supports it)", cmd_clear},
    {"reboot", "reboot",     "reboot the Mango Pi", cmd_reboot},
    {"peek", "peek [addr]",  "print contents of memory at address", cmd_peek}, 
    {"poke", "poke [addr] [val]",  "store value into memory at address", cmd_poke},
    {"history", "history",  "prints the 10 most recently executed commands", cmd_history},
    {"!!", "!!",  "repeat the last command", cmd_repeat},
    {"disassemble", "disassemble [addr]", "disassemble instruction at address", cmd_disassemble} 
};

int cmd_disassemble(int argc, const char **argv) {
    if (argc < 2) {
        module.shell_printf("error: disassemble expects 1 argument [addr]\n");
        return 1;
    }

    const char *end;
    unsigned long addr = strtonum(argv[1], &end);

    if (*end != '\0') {
        module.shell_printf("error: cannot convert '%s'\n", argv[1]);
        return 1;
    }

    if (addr % 4 != 0) {
        module.shell_printf("error: address must be 4-byte aligned\n");
        return 1;
    }

    unsigned int *ptr = (unsigned int *)addr;  // cast to pointer
    module.shell_printf("%pI\n", (void *)ptr); // cast to void* for %p

    return 0;
}

void save_to_history(int argc, const char* argv[]) {
    circularHistory *historyAddress = &commandHistory;
                
    if (historyAddress->head == -1) {
        // First command
        historyAddress->head = 0;
        historyAddress->tail = 0;
        historyAddress->count = 1;
        historyAddress->countOfAllEntries = 1;
    } else {
        // Move head forward
        historyAddress->head = (historyAddress->head + 1) % MAX_HISTORY;

        historyAddress->countOfAllEntries++;
        if (historyAddress->count < MAX_HISTORY) {
            historyAddress->count++;
        } else {
            historyAddress->tail = (historyAddress->tail + 1) % MAX_HISTORY; // overwrite oldest
        }
    }

    // Copy argv into the history at the tail command index
    char *historyIndexAddress = historyAddress->history[historyAddress->head];
    size_t pos = 0;
    for (int i = 0; i < argc; i++) {
        size_t len = strlen(argv[i]);
        memcpy(historyIndexAddress + pos, argv[i], len);
        pos += len;
        historyIndexAddress[pos++] = ' ';  // space between argv
    }
    historyIndexAddress[pos - 1] = '\0';  // overwrite last space
}

// Returns 0 on success or 1 on failure
int cmd_history(int argc, const char *argv[]) {
    save_to_history(argc, argv);
    circularHistory *historyAddress = &commandHistory;

    int index = historyAddress->tail;
    int startNum = historyAddress->countOfAllEntries - historyAddress->count + 1; // number of oldest command
    // calculate number of digits in largest number
    int tmp = historyAddress->countOfAllEntries;
    int maxDigits = 1;
    while (tmp >= 10) {
        tmp /= 10;
        maxDigits++;
    }
    
    for (int i = 0; i < historyAddress->count; i++) { 
        int curNum = startNum + i;

        // compute number of digits in current number
        int tmp2 = curNum;
        int curDigits = 1;
        while (tmp2 >= 10) {
            tmp2 /= 10;
            curDigits++;
        }

        // print spaces BEFORE the digit to pad to maxDigits
        for (int j = 0; j < (maxDigits - curDigits); j++) {
            module.shell_printf(" ");
        }
        module.shell_printf("%d", startNum + i);
        module.shell_printf(" ");
        module.shell_printf("%s\n", historyAddress->history[index]);
        index = (index + 1) % MAX_HISTORY;
    }

    return 0;
}

int cmd_repeat(int argc, const char *argv[]) {
    if (previousCommand[0] == '\0') {
        module.shell_printf("error: no command to repeat\n");
        return 1;
    }

    module.shell_printf("%s\n", previousCommand); // optional
    return shell_evaluate(previousCommand);
}

int cmd_poke(int argc, const char *argv[]) {
    if (argc < 3) {
        module.shell_printf("error: poke expects 2 arguments [addr] and [val]\n");
        return 1;
    }

    const char *end1;
    unsigned long address = strtonum(argv[1], &end1);

    if (*end1 != '\0') {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[1]);
        return 1;
    }

    const char *end2;
    unsigned long inputValue = strtonum(argv[2], &end2);

    if (*end2 != '\0') {
        module.shell_printf("error: poke cannot convert '%s'\n", argv[2]);
        return 1;
    }
    
    if (address % 4 != 0) {
        module.shell_printf("error: poke address must be 4-byte aligned\n");
        return 1;
    }

    *(unsigned int *)address = inputValue;

    return 0;    
}

int cmd_peek(int argc, const char *argv[]) {
    if (argc < 2) {
        module.shell_printf("error: peek expects 1 argument [addr]\n");
        return 1;
    }

    const char *end;
    unsigned long address = strtonum(argv[1], &end);

    if (*end != '\0') {
        module.shell_printf("error: peek cannot convert '%s'\n", argv[1]);
        return 1;
    }
    
    if (address % 4 != 0) {
        module.shell_printf("error: peek address must be 4-byte aligned\n");
        return 1;
    }

    unsigned int value = *(unsigned int *)address;
    
    module.shell_printf("%p: %08x\n", (unsigned int *)address, value);

    return 0;
}

int cmd_reboot(int argc, const char *argv[]) {
    module.shell_printf("Rebooting...\n");
    mango_reboot();
}

int cmd_echo(int argc, const char *argv[]) {
    for (int i = 1; i < argc; ++i)
        module.shell_printf("%s ", argv[i]);
    module.shell_printf("\n");
    return 0;
}

int cmd_help(int argc, const char *argv[]) {
    size_t num_commands = sizeof(commands) / sizeof(commands[0]);

    if (argc == 1) { 
        for (size_t i = 0; i < num_commands; i++) {
            int pad = MAX_COMMAND_LENGTH - strlen(commands[i].usage);
            module.shell_printf("%s", commands[i].usage);
            
            for (int j = 0; j < pad; j++) { 
                module.shell_printf(" ");
            
            }
            module.shell_printf("%s\n", commands[i].description);
        }
    } else { 
        int commandTableIndex = -1;

        for (size_t i = 0; i < num_commands; i++) {
            if (strcmp(commands[i].name, argv[1]) == 0) {
                commandTableIndex = i;
            }
        }

        if (commandTableIndex == -1) { 
            module.shell_printf("error: no such command '%s'\n", argv[1]);
            return 1;
        }
        
        int pad = MAX_COMMAND_LENGTH - strlen(commands[commandTableIndex].usage);
        module.shell_printf("%s", commands[commandTableIndex].usage);
        
        for (int j = 0; j < pad; j++) {
            module.shell_printf(" ");
        }

        module.shell_printf("%s\n", commands[commandTableIndex].description);
    }
    return 0;
}

int cmd_clear(int argc, const char* argv[]) {
    module.shell_printf("\f");   // formfeed character
    return 0;
}

void shell_init(input_fn_t read_fn, formatted_fn_t print_fn) {
    module.shell_read = read_fn;
    module.shell_printf = print_fn;
}

void shell_bell(void) {
    uart_putchar('\a');
}

void shell_readline(char buf[], size_t bufsize) {
    char curChar;
    int index = 0;
    int length = 0;
    circularHistory *historyAddress = &commandHistory;
    int historyPosition = -1; // current position in history
    bool usingHistory = false;
    
    while (true) {
        curChar = module.shell_read();
        
        // Ctrl-L
        if (curChar == 12) {
            module.shell_printf("\f\n");
            module.shell_printf("Pi> ");

            // redraw current buffer
            for (int i = 0; i < length; i++) {
                module.shell_printf("%c", buf[i]);
            }
            // restore cursor position
            for (int i = length; i > index; i--) {
                module.shell_printf("\b");
            }

            continue;
        }
        
        // Ctrl-A
        if (curChar == 1) {
            index = 0;
            module.shell_printf("\rPi> ");
            continue;
        }

        // Ctrl-E
        if (curChar == 5) {
            index = length;
            module.shell_printf("\rPi> "); // go to prompt
            for (int i = 0; i < length; i++) {
                module.shell_printf("%c", buf[i]);
            }   
            continue;
        }

        // Stop when return
        if (curChar == '\n') {
            module.shell_printf("\n");
            break;
        }
        
        if (curChar == 0xaa) {
            if (!usingHistory) {
                // start at most recent command
                if (historyAddress->count == 0) {
                    shell_bell(); // no history
                    continue;
                }
                historyPosition = historyAddress->head;
                usingHistory = true;
            } else if (historyPosition == historyAddress->tail) {
                shell_bell(); // reached oldest
                continue;
            } else {
                historyPosition = (historyPosition - 1 + MAX_HISTORY) % MAX_HISTORY;
            }

            int oldLength = length;   // save previous line length

            size_t commandLength = strlen(historyAddress->history[historyPosition]);

            // copy history into buffer
            memcpy(buf, historyAddress->history[historyPosition], commandLength);
            buf[commandLength] = '\0';

            length = commandLength;
            index = length;

            // redraw line
            module.shell_printf("\rPi> ");

            // print new command
            for (int i = 0; i < length; i++) {
                module.shell_printf("%c", buf[i]);
            }

            // erase leftover characters from previous longer line
            for (int i = length; i < oldLength; i++) {
                module.shell_printf(" ");
            }

            // move cursor back if we erased characters
            for (int i = oldLength; i > length; i--) {
                module.shell_printf("\b");
            }
            continue;
        }

        // Down Arrow
        if (curChar == 0xab) {
            if (!usingHistory) {
                shell_bell(); // not in history, can't go down
                continue;
            } else if (historyPosition == historyAddress->head) {
                shell_bell(); // reached newest
                continue;
            } else {
                historyPosition = (historyPosition + 1) % MAX_HISTORY;
            }

            int oldLength = length;   // save previous line length

            size_t commandLength = strlen(historyAddress->history[historyPosition]);

            // copy history into buffer
            memcpy(buf, historyAddress->history[historyPosition], commandLength);
            buf[commandLength] = '\0';

            length = commandLength;
            index = length;

            // redraw line
            module.shell_printf("\rPi> ");

            // print new command
            for (int i = 0; i < length; i++) {
                module.shell_printf("%c", buf[i]);
            }

            // erase leftover characters from previous longer line
            for (int i = length; i < oldLength; i++) {
                module.shell_printf(" ");
            }

            // move cursor back if we erased characters
            for (int i = oldLength; i > length; i--) {
                module.shell_printf("\b");
            }
            continue;
        }

        // Left Arrow 
        if (curChar == 0xac) {
            if (index == 0) {
                shell_bell();
            } else {
                index--;
                module.shell_printf("\b");
            }
            continue;
        }

        // Right Arrow
        if (curChar == 0xad) {
            if (index == length) {
                shell_bell();
            } else {
                index++;
                module.shell_printf("\033[1C");
            }
            continue;
        }

        // reject non-ascii
        if (curChar > 0x7f) {
            shell_bell();
            continue;
        }
        
        // handle backspace
        if (curChar == '\b') {
            usingHistory = false;
            historyPosition = historyAddress->head;
            // Prevent backspace when no char in buffer
            if (index == 0) {
                shell_bell();
            } else {
                for (int i = index - 1; i < length - 1; i++) {
                    buf[i] = buf[i+1];
                }

                index--;
                length--;

                // redraw line
                module.shell_printf("\rPi> ");
                for (int i = 0; i < length; i++) {
                    module.shell_printf("%c", buf[i]);
                }
                module.shell_printf(" ");
                module.shell_printf("\b"); 
                
                // move index to correct position
                for (int i = length; i > index; i--) {
                    module.shell_printf("\b"); 
                }
            }
            continue;
        }

        // prevent buffer overflow
        if (length >= bufsize - 1) {
            shell_bell();
            continue;
        }

        // store + echo character
        if (curChar > 26) { // Ignore control char
            usingHistory = false;
            historyPosition = historyAddress->head;
            if (index == length) {
                // typing at end — fast path
                buf[index++] = curChar;
                length++;
                module.shell_printf("%c", curChar);
                continue;
            }

            // Shift characters at cursor to the right
            for (int i = length; i > index; i--) {
                buf[i] = buf[i-1];
            }

            buf[index] = curChar;
            index++;
            length++;
            module.shell_printf("\rPi> ");

            for (int i = 0; i < length; i++) {
                module.shell_printf("%c", buf[i]);
            }
            // Move cursor back
            for (int i = length; i > index; i--) {
                module.shell_printf("\b"); 
            }
        }
    }
    
    buf[length] = '\0';
}

bool is_whitespace(char ch) {
    return (ch == ' ') || (ch == '\n') || (ch == '\t');
}

int shell_evaluate(const char *line) {
    // Parse line
    char *tokens[MAX_ARGS];
    int tokenIndex = 0;

    // Create copy of input 
    size_t inputSize = strlen(line) + 1;
    char commandInputs[inputSize];
    memcpy(commandInputs, line, inputSize);
    
    // Tokenize input
    bool inToken = false;

    for (int i = 0; commandInputs[i] != '\0'; i++) {
        if (!is_whitespace(commandInputs[i])) {
            if (!inToken) {
                // Start of a new token
                if (tokenIndex < MAX_ARGS)
                    tokens[tokenIndex++] = &commandInputs[i];
                inToken = true;
            }
        } else {
            if (inToken) {
                // End the current token
                commandInputs[i] = '\0';
                inToken = false;
            }
        }
    } 

    if (tokenIndex == 0) {
        return -1;  // stop further processing
    }

    // Execute command
    // Size of array / size of array element
    size_t num_commands = sizeof(commands) / sizeof(commands[0]);
    
    for (size_t i = 0; i < num_commands; i++) {
        if (strcmp(commands[i].name, tokens[0]) == 0) {
            //for (int i = 0; i < tokenIndex; i++)
            //    module.shell_printf("[%s]\n", tokens[i]);
            int result = commands[i].fn(tokenIndex, (const char **)tokens);

            if (strcmp(commands[i].name, "history") != 0 && strcmp(commands[i].name, "!!") != 0) {
                save_to_history(tokenIndex, (const char **)tokens);
            }

            return result;
        }
    }

    module.shell_printf("error: no such command %s\n", tokens[0]);
    return -1;
}

void shell_saveLine(const char *line, size_t n) {
    memcpy(previousCommand, line, n);
    previousCommand[n] = '\0';
}

void shell_run(void) {
    module.shell_printf("Welcome to the CS107E shell.\nRemember to type on your PS/2 keyboard!\n");
    while (1) {
        char line[LINE_LEN];

        module.shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        if (!(line[0] == '!' && line[1] == '!')) {
            shell_saveLine(line, strlen(line));
        }
        shell_evaluate(line);
    }
}
