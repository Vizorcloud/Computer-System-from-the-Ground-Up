/* File: backtrace
 * -----------------
 * Purpose: Backtrace Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 12 2025
 */
#include "backtrace.h"
#include "mango.h"
#include "printf.h"
#include "symtab.h"

// helper function implemented in file backtrace_asm.s
extern unsigned long backtrace_get_fp(void);

int backtrace_gather_frames(frame_t f[], int max_frames) {
    // Set current fp and the count
    uintptr_t *fp = (uintptr_t *)backtrace_get_fp();
    int count = 0;
    
    // Iterate until the stack floor is hit or we've harvested enough frames
    while (fp && count < max_frames) {
        // Create a temp frame with the value of the saved ra
        frame_t frame;
        frame.resume_addr = fp[-1];
        f[count] = frame;

        count++;
        
        // Set fp to the value of the saved fp in the callee
        fp = (uintptr_t *)fp[-2];  
    }

    return count;
}

void backtrace_print_frames(frame_t f[], int n) {
    char labelbuf[128];

    for (int i = 0; i < n; i++) {
        symtab_label_for_addr(labelbuf, sizeof(labelbuf), f[i].resume_addr);
        printf("#%d 0x%08lx at %s\n", i, f[i].resume_addr, labelbuf);
    }
}

void backtrace_print(void) {
    int max = 50;
    frame_t arr[max];

    int n = backtrace_gather_frames(arr, max);
    backtrace_print_frames(arr+1, n-1);   // print frames starting at this function's caller
}

long __stack_chk_guard = 0x0f1e2d3c4b5a6978;

void __stack_chk_fail(void)  {
    uintptr_t *fp = (uintptr_t *)backtrace_get_fp();
    //printf("Initial fp from backtrace_get_fp(): %p\n", fp);
    fp = (uintptr_t *)fp[-2]; // walk one frame up past __stack_chk_fail helper
    //printf("Frame above __stack_chk_fail: %p\n",fp);
    uintptr_t ra = fp[-1];    // RA of the bad actor 
    //printf("Saved RA of bad actor: 0x%lx\n", ra);
    symbol_t sym;
    const char *fn_name;

    if (symtab_symbol_for_addr(ra, &sym)) {
        fn_name = sym.name;
    } else {
        fn_name = "<unknown>";
    }

    printf("\n *** Stack smashing detected at end of function %s() *** \n", fn_name);
    mango_abort();
}
