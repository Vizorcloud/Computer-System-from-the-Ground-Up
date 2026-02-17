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
#define MAX_ARGS 50
#define MAX_COMMAND_LENGTH 20

// Module-level global variables for shell
static struct {
    input_fn_t shell_read;
    formatted_fn_t shell_printf;
} module;


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
    {"poke", "poke [addr] [val]",  "store value into memory at address", cmd_poke}
};

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
    
    while (true) {
        curChar = module.shell_read();
        
        // Stop when return
        if (curChar == '\n') {
            module.shell_printf("\n");
            break;
        }

        // reject non-ascii
        if (curChar > 0x7f) {
            shell_bell();
            continue;
        }
        
        // handle backspace
        if (curChar == '\b') {
            // Prevent backspace when no char in buffer
            if (index == 0) {
                shell_bell();
            } else {
                index--;
                module.shell_printf("%c %c", '\b', '\b');
            }
            continue;
        }

        // prevent buffer overflow
        if (index >= bufsize - 1) {
            shell_bell();
            continue;
        }

        // store + echo character
        buf[index] = curChar;
        index++;
        module.shell_printf("%c", curChar);
    }
    
    buf[index] = '\0';
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
    // Execute command
    // Size of array / size of array element
    size_t num_commands = sizeof(commands) / sizeof(commands[0]);
    
    for (size_t i = 0; i < num_commands; i++) {
        if (strcmp(commands[i].name, tokens[0]) == 0) {
            //for (int i = 0; i < tokenIndex; i++)
            //    module.shell_printf("[%s]\n", tokens[i]);
            return commands[i].fn(tokenIndex, (const char **)tokens);
        }
    }

    module.shell_printf("error: no such command %s\n", tokens[0]);
    return 1;
}

void shell_run(void) {
    module.shell_printf("Welcome to the CS107E shell.\nRemember to type on your PS/2 keyboard!\n");
    while (1) {
        char line[LINE_LEN];

        module.shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
