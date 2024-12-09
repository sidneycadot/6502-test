
//////////////////////
// target_sim6502.c //
//////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <peekpoke.h>

#include "target.h"

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
    printf("Test: %s ...\n", test_description);
}

bool post_every_measurement_hook(const char * test_description, bool success, unsigned long test_count, unsigned long error_count)
{
    (void)test_description;
    (void)success;
    (void)test_count;
    (void)error_count;

    return true;
}

bool zp_address_is_safe(uint8_t address)
{
    (void)address;
    return true;
}

uint8_t * set_irq_vector_address(uint8_t * newvec)
{
    uint8_t * oldvec = (uint8_t *)PEEKW(0xfffe);
    POKEW(0xfffe, (uint16_t)newvec);
    return oldvec;
}
