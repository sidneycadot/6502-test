
////////////////////
// target_atari.c //
////////////////////

#include <stdbool.h>
#include <stdint.h>
#include <peekpoke.h>

#include "target.h"

#define POKMSK  0x0010
#define VIMIRQ  0x0216
#define SDMCTL  0x022f
#define COLBK   0xd01a
#define IRQEN   0xd20e
#define DMACTL  0xd400
#define NMIEN   0xd40e

void measurement_live_report(const char * test_description, unsigned long test_count, bool success)
{
    (void)test_description;
    (void)success;
    POKE(COLBK, test_count << 1);
}

bool zp_address_is_safe(uint8_t address)
{
    (void)address;
    return true; // On the atari, all zero-page addresses are freely usable.
}

void dma_and_interrupts_off(void)
{
    POKE(NMIEN, 0);
    POKE(IRQEN, 0);
    POKE(DMACTL, 0);
}

void dma_and_interrupts_on(void)
{
    POKE(DMACTL, PEEK(SDMCTL));
    POKE(IRQEN, PEEK(POKMSK));
    POKE(NMIEN, 0x40);
}

uint8_t * set_irq_vector_address(uint8_t * newvec)
{
    uint8_t * oldvec = PEEKW(VIMIRQ);
    POKEW(VIMIRQ, newvec);
    return oldvec;
}
