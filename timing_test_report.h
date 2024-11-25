
//////////////////////////
// timing_test_report.h //
//////////////////////////

#ifndef TIMING_TEST_REPORT_H
#define TIMING_TEST_REPORT_H

void reset_test_counts(void);
void test_report(const char * test_description, unsigned test_overhead_cycles, unsigned instruction_cycles, unsigned actual_cycles, ...);

extern unsigned long error_count;
extern unsigned long test_count;

#endif
