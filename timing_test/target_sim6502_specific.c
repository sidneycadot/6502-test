
/////////////////////////////
// target_sim65_specific.c //
/////////////////////////////

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

void pre_every_test_hook(const char * opcode_description)
{
    printf("Testing opcode: %s ...\n", opcode_description);
}

bool post_every_measurement_hook(bool success, unsigned opcode_count, unsigned long measurement_count, unsigned long error_count)
{
    (void)success;
    (void)opcode_count;
    (void)measurement_count;
    (void)error_count;
    return true;
}

uint8_t * set_irq_vector_address(uint8_t * newvec)
{
    uint8_t * oldvec = (uint8_t *)PEEKW(0xfffe);
    POKEW(0xfffe, (uint16_t)newvec);
    return oldvec;
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
