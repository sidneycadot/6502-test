
///////////////////////////////////
// timing_test_zeropage_backup.c //
///////////////////////////////////

#include <peekpoke.h>
#include <stdint.h>

#include "target.h"
#include "timing_test_zeropage_backup.h"

static uint8_t zpage_backup[0x256];

void save_zero_page(void)
{
    unsigned zp_address;
    for (zp_address = 0; zp_address <= 0xff; ++zp_address)
    {
        if (zp_address_is_safe(zp_address))
        {
            zpage_backup[zp_address] = PEEK(zp_address);
        }
    }
}

void restore_zero_page(void)
{
    unsigned zp_address;
    for (zp_address = 0; zp_address <= 0xff; ++zp_address)
    {
        if (zp_address_is_safe(zp_address))
        {
            POKE(zp_address, zpage_backup[zp_address]);
        }
    }
}
