
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

void pre_opcode_hook(const char * opcode_description, bool skip_flag)
{
    if (skip_flag)
    {
        printf("Skipping opcode: %s ...\n", opcode_description);
    }
    else
    {
        printf("Testing opcode: %s ...\n", opcode_description);
    }
}

bool post_every_measurement_hook(bool success, unsigned opcode_count, unsigned long measurement_count, unsigned long error_count)
{
    (void)success;
    (void)opcode_count;
    (void)measurement_count;
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

uint8_t get_cpu_signature(void)
{
    return 3;
}
