
////////////////////
// target_atari.c //
////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <peekpoke.h>

#include "target.h"

#define POKMSK  0x0010
#define LMARGN  0x0052
#define VIMIRQ  0x0216
#define SDMCTL  0x022f
#define COLOR2  0x02c6
#define COLOR4  0x02c8
#define COLBK   0xd01a
#define IRQEN   0xd20e
#define DMACTL  0xd400
#define NMIEN   0xd40e


void program_start_hook(void)
{
    POKE(LMARGN, 0);
    POKE(COLOR2, 0);
    POKE(COLOR4, 182);
    putchar(125); // Clear screen.
}

void program_end_hook(void)
{
}

void post_measurement_cycles_hook(const char * test_description, bool success, unsigned long test_count, unsigned long error_count)
{
    (void)test_description;
    (void)success;
    (void)error_count;
    POKE(COLBK, test_count << 1);
}

bool zp_address_is_safe(uint8_t address)
{
    (void)address;
    return true; // On the atari, all zero-page addresses are freely usable.
}

void pre_measurements_hook(void)
{
    // Disable NMI interrupts, IRQ interrupts, and video DMA.
    POKE(NMIEN, 0);
    POKE(IRQEN, 0);
    POKE(DMACTL, 0);
}

void post_measurements_hook(void)
{
    // Re-enable video DMA, IRQ interrupts, and NMI (VBLANK) interrupts.
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
