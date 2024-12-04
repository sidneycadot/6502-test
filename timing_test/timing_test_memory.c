
//////////////////////////
// timing_test_memory.c //
//////////////////////////

#include <stddef.h>
#include <stdlib.h>

#include "timing_test_memory.h"

uint8_t * TESTCODE_PTR    = NULL; // The pointer to the full test area, allocated using malloc().
uint8_t * TESTCODE_BASE   = NULL; // The first address in the TESTCODE range that is on a page boundary.
uint8_t * TESTCODE_ANCHOR = NULL; // The halfway point in the TESTCODE range, also on a page boundary. Put test code here.

int allocate_testcode_block(unsigned size)
{
    unsigned block_size, offset;

    if (size % 512 != 0)
    {
        // We are only willing to allocate an even number of pages; report failure.
        return -1;
    }

    // We want a paged-aligned block of size 'size'.
    // To guarantee that we get that, allocate 255 bytes more.

    block_size = size + 255;

    TESTCODE_PTR = malloc(block_size);
    if (TESTCODE_PTR == NULL)
    {
        // Unable to allocate the required memory; report failure.
        return -1;
    }

    // Where does the first page start?

    offset = (256 - (unsigned)TESTCODE_PTR % 256) % 256;

    // Initialize the important variables.

    TESTCODE_BASE   = TESTCODE_PTR + offset;
    TESTCODE_ANCHOR = TESTCODE_BASE + size / 2;

    return 0;
}

void free_testcode_block(void)
{
    free(TESTCODE_PTR);
}
