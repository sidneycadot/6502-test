
//////////////////////////
// timing_test_memory.h //
//////////////////////////

#ifndef TIMING_TEST_MEMORY_H
#define TIMING_TEST_MEMORY_H

#include <stdint.h>

extern uint8_t * TESTCODE_PTR;    // The pointer to the full test area, as allocated using malloc().
extern uint8_t * TESTCODE_BASE;   // The first address in the TESTCODE range that is on a page boundary.
extern uint8_t * TESTCODE_ANCHOR; // The halfway point in the TESTCODE range, also on a page boundary. Put test code here.

int allocate_testcode_block(unsigned size);
void free_testcode_block(void);

#endif
