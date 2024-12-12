
///////////////////////////////
// timing_test_measurement.h //
///////////////////////////////

#ifndef TIMING_TEST_MEASUREMENT_H
#define TIMING_TEST_MEASUREMENT_H

#include <stdbool.h>
#include <stdint.h>

extern unsigned long test_count;
extern unsigned long error_count;

#define F_NONE             0
#define F_STOP_ON_ERROR    0x01

typedef enum {
    Par1,
    Par12,
    Par123,
    Par1234
} LoopSpec;

void reset_test_counts(void);
bool run_measurement(const char * test_description, unsigned test_overhead_cycles, unsigned instruction_cycles, uint8_t * entrypoint, uint8_t flags, LoopSpec loopspec);
void report_test_counts(void);

#endif
