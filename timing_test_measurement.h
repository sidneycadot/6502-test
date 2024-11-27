
///////////////////////////////
// timing_test_measurement.h //
///////////////////////////////

#ifndef TIMING_TEST_MEASUREMENT_H
#define TIMING_TEST_MEASUREMENT_H

#include <stdbool.h>
#include <stdint.h>

void reset_test_counts(void);
bool run_measurement(const char * test_description, unsigned test_overhead_cycles, unsigned instruction_cycles, uint8_t * entrypoint, bool save_zp_flag, ...);
void report_test_counts(void);

#endif
