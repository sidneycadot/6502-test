
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
#define COLOR1  0x02c5
#define COLOR2  0x02c6
#define COLOR4  0x02c8
#define HPOSM0  0xd004
#define HPOSM1  0xd005
#define GRAFM   0xd011
#define COLPM0  0xd012
#define COLPM1  0xd013
#define COLBK   0xd01a
#define CONSOL  0xd01f
#define IRQEN   0xd20e
#define DMACTL  0xd400
#define NMIEN   0xd40e

static unsigned progress1; // test types
static unsigned progress2; // tests

static uint8_t SAVE_LMARGN;
static uint8_t SAVE_COLOR1;
static uint8_t SAVE_COLOR2;
static uint8_t SAVE_COLOR4;

void program_start_hook(void)
{
    SAVE_LMARGN = PEEK(LMARGN);
    SAVE_COLOR1 = PEEK(COLOR1);
    SAVE_COLOR2 = PEEK(COLOR2);
    SAVE_COLOR4 = PEEK(COLOR4);

    POKE(LMARGN, 0);
    POKE(COLOR1, 14);
    POKE(COLOR2, 116);
    POKE(COLOR4, 112);
    putchar(125); // Clear screen.
}

void program_end_hook(void)
{
    POKE(LMARGN, SAVE_LMARGN);
    POKE(COLOR1, SAVE_COLOR1);
    POKE(COLOR2, SAVE_COLOR2);
    POKE(COLOR4, SAVE_COLOR4);
}

void pre_big_measurement_block_hook(void)
{
    // Disable NMI interrupts, IRQ interrupts, and video DMA.
    POKE(NMIEN, 0);
    POKE(IRQEN, 0);
    POKE(DMACTL, 0);

    POKE(GRAFM, 0x55);
    POKE(COLPM0, 202);
    POKE(COLPM1, 14);
    progress1 = 43;
    progress2 = 43;
}

void post_big_measurement_block_hook(void)
{
    POKE(GRAFM, 0);
    POKE(COLPM0, 0);
    POKE(COLPM1, 0);
    POKE(HPOSM0, 0);
    POKE(HPOSM1, 0);

    // Re-enable video DMA, IRQ interrupts, and NMI (VBLANK) interrupts.
    POKE(DMACTL, PEEK(SDMCTL));
    POKE(IRQEN, PEEK(POKMSK));
    POKE(NMIEN, 0x40);
}

void pre_every_test_hook(const char * test_description)
{
    (void)test_description;
    POKE(HPOSM0, progress1);
    if (progress1 == 210)
    {
        progress1 = 43;
    }
    else
    {
        ++progress1;
    }
}

bool post_every_measurement_hook(const char * test_description, bool success, unsigned long test_count, unsigned long error_count)
{
    (void)test_description;
    (void)success;
    (void)test_count;
    (void)error_count;
    POKE(HPOSM1, progress2);
    if (progress2 == 210)
    {
        progress2 = 43;
    }
    else
    {
        ++progress2;
    }

    return PEEK(CONSOL) == 7; // Continue if no buttons have been pushed.
}


uint8_t * set_irq_vector_address(uint8_t * newvec)
{
    uint8_t * oldvec = (uint8_t *)PEEKW(VIMIRQ);
    POKEW(VIMIRQ, (uint16_t)newvec);
    return oldvec;
}

bool zp_address_is_safe(uint8_t zp_address)
{
    (void)zp_address;
    return true;
}
