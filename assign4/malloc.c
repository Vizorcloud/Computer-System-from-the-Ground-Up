/* File: malloc.c
 * --------------
 * ***** TODO: add your file header comment here *****
 */


 /*
 * The code given below is simple "bump" allocator from lecture.
 * An allocation request is serviced by using sbrk to extend
 * the heap segment.
 * It does not recycle memory (free is a no-op) so when all the
 * space set aside for the heap is consumed, it will not be able
 * to service any further requests.
 *
 * This code is given here just to show the very simplest of
 * approaches to dynamic allocation. You will replace this code
 * with your own heap allocator implementation.
 */

#include "malloc.h"
#include "memmap.h"
#include "printf.h"
#include <stddef.h> // for NULL
#include "strings.h"

/*
 * Data variables private to this module used to track
 * statistics for debugging/validate heap:
 *    count_allocs, count_frees, total_bytes_requested
 */
static int count_allocs, count_frees, total_bytes_requested;

/*
 * The segment of memory available for the heap runs from HEAP_START
 * to HEAP_MAX (markers placed in memmap.ld establish these boundaries,
 * constants declared in memmap.h)
 *
 * The pointer variable cur_head_end is initialized to HEAP_START and
 * is adjusted upward as in-use portion of heap segment enlarges.
 * Because cur_head_end is qualified as static, this variable
 * is not stored in stack frame, instead variable is located in data segment.
 * The one variable is shared by all and retains its value between calls.
 */

// Call sbrk to enlarge in-use heap area
void *sbrk(size_t nbytes) {
    static void *cur_heap_end = HEAP_START;     // IMPORTANT: declared static

    void *new_heap_end = (char *)cur_heap_end + nbytes;
    if (new_heap_end > HEAP_MAX)    // if request would extend beyond heap max
        return NULL;                // reject
    void *prev_heap_end = cur_heap_end;
    cur_heap_end = new_heap_end;
    return prev_heap_end;
}

// Macro to round up x to multiple of n.
// The efficient but tricky bitwise approach it uses
// works only if n is a power of two -- why?
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

void *malloc (size_t nbytes) {
    /***** TODO: Replace with your code *****/
    count_allocs++;
    total_bytes_requested += nbytes;
    nbytes = roundup(nbytes, 8);
    return sbrk(nbytes);
}

void free (void *ptr) {
    count_frees++;
    /***** TODO: Your code goes here *****/
}

void heap_dump (const char *label) {
    void *cur_heap_end = sbrk(0);
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    printf("Heap segment at %p - %p\n", HEAP_START, cur_heap_end);

    /***** TODO: Your code goes here *****/

    printf("----------  END DUMP (%s) ----------\n", label);
    printf("Stats: %d in-use (%d allocs, %d frees), %d total payload bytes requested\n\n",
        count_allocs - count_frees, count_allocs, count_frees, total_bytes_requested);
}

void malloc_report (void) {
    printf("\n=============================================\n");
    printf(  "         Mini-Valgrind Malloc Report         \n");
    printf(  "=============================================\n");
    /***** TODO EXTENSION: Your code goes here if implementing extension *****/
}

void report_damaged_redzone (void *ptr) {
    printf("\n=============================================\n");
    printf(  " **********  Mini-Valgrind Alert  ********** \n");
    printf(  "=============================================\n");
    printf("Attempt to free address %p that has damaged red zone(s):", ptr);
    /***** TODO EXTENSION: Your code goes here if implementing extension *****/
}
