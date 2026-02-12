/* File: malloc.c
 * --------------
 * Purpose: Malloc Module Library Implementation
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Feb 12 2025
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

typedef struct {
    int payload_size;
    int status;       // 0 if free, 1 if in use
} block_header_t;

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
    // update tracker variables for debugging
    count_allocs++;
    total_bytes_requested += nbytes;
    
    void *heap_end = sbrk(0);
    block_header_t *cur_header = (block_header_t *)HEAP_START;
    size_t payload_size = roundup(nbytes, 8);

    while ((void *)cur_header < heap_end) {
        // If we find a recycleable block
        if (cur_header->status == 0 && cur_header->payload_size >= payload_size) {
            size_t leftoverBytes = cur_header->payload_size - payload_size;
            size_t minBlockSize = 8 + sizeof(block_header_t);
            
            // If there is enough space for a new free block we split to create one
            if (leftoverBytes >= minBlockSize) {
                block_header_t *newblockheader = (block_header_t *)((char *)(cur_header + 1) + payload_size);
                newblockheader->status = 0;
                newblockheader->payload_size = leftoverBytes - sizeof(block_header_t);

                cur_header->payload_size = payload_size;  
            }

            cur_header->status = 1;

            return (void *)(cur_header + 1);
        }
        // Header size (in 8 bytes) + payload size (in bytes)
        cur_header = (block_header_t *)((char *)(cur_header + 1) + cur_header->payload_size);
    }
    
    // create space for client requested bytes + header
    size_t block_size = payload_size + sizeof(block_header_t); 

    // Fill the header
    block_header_t *headerPtr = sbrk(block_size);
    // Guard for if sbrk() returns NULL when there is not enough heap space
    if (!headerPtr) return NULL;
    
    headerPtr->payload_size = payload_size;
    headerPtr->status = 1;
    
    // block_header_t is 8 bytes long
    return (void *)(headerPtr + 1);
}

void free (void *ptr) {
    // Update tracker variables for debugging
    count_frees++;

    // Guard for nullptr
    if (!ptr) return;
    
    // Free the memory
    void *heap_end = sbrk(0);
    block_header_t *cur_header = (block_header_t *)ptr - 1; 
    block_header_t *next_header = (block_header_t *)((char *)(cur_header + 1) + cur_header->payload_size);

    while ((void *)next_header < heap_end && next_header->status == 0) {
        size_t blockSize = next_header->payload_size + sizeof(block_header_t);
        cur_header->payload_size += blockSize;
        next_header = (block_header_t *)((char *)(next_header + 1) + next_header->payload_size);
    }
    
    cur_header->status = 0;
}

void heap_dump (const char *label) {
    void *cur_heap_end = sbrk(0);
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    printf("Heap segment at %p - %p\n", HEAP_START, cur_heap_end);

    // Traverse the heap
    block_header_t *ptr = (block_header_t *)HEAP_START;

    while ((void *)ptr < cur_heap_end) {
        // Print payload info
        void *payloadPtr = ptr + 1;    
        printf("----------------------\n");
        printf("Payload address: %p\n", payloadPtr);
        printf("Payload size: %d\n", ptr->payload_size);
        printf("Payload status: %d\n", ptr->status);
        
        // Print up to first 16 bytes of payload
        for (size_t i = 0; i < 16 && i < ptr->payload_size; i++)
            printf("%02x ", ((unsigned char *)payloadPtr)[i]);
        printf("\n");

        printf("----------------------\n");

        // Header size (in 8 bytes) + payload size (in bytes)
        ptr = (block_header_t *)((char *)payloadPtr + ptr->payload_size);
    }

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
