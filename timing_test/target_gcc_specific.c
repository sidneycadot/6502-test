
///////////////////////////
// target_gcc_specific.c //
///////////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "target.h"
#include "timing_test_measurement.h"

void program_start_hook(void)
{
}

void program_end_hook(void)
{
}

void pre_big_measurement_block_hook(void)
{
}

void post_big_measurement_block_hook(void)
{
}

void pre_every_test_hook(const char * test_description)
{
    printf("Running test: %s ...\n", test_description);
}

bool post_every_measurement_hook(const char * test_description, bool success, unsigned long test_count, unsigned long msm_count, unsigned long error_count)
{
    (void)test_description;
    (void)success;
    (void)test_count;
    (void)msm_count;
    (void)error_count;
    return true; // Proceed.
}


uint8_t * set_irq_vector_address(uint8_t * newvec)
{
    return newvec;
}

bool zp_address_is_safe_for_read(uint8_t zp_address)
{
    (void)zp_address;
    return true;
}

bool zp_address_is_safe_for_write(uint8_t zp_address)
{
    (void)zp_address;
    return true;
}

int16_t measure_cycles_wrapper(uint8_t * code)
{
    (void)code;
    return m_test_overhead_cycles + m_instruction_cycles;
}
