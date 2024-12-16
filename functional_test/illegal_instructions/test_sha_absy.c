
/////////////////////
// test_sha_absy.c //
/////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <peekpoke.h>

void __fastcall__ sha_absy(uint16_t base, uint8_t ra, uint8_t rx, uint8_t ry);

int main(void)
{
    uint16_t base;
    uint16_t eff_addr;
    uint8_t ra, rx, ry;
    uint8_t before, after, predicted_after;
    unsigned long count, count_bad;
    bool ok;

    for (count = count_bad=0;;++count)
    {
        if (count % 10 == 0)
        {
            printf("count: %lu bad: %lu\n", count, count_bad);
        }

        // 0x4000 .. 0x5100 is safe for tests an the Atari.
        base = 0x4000 + (rand() & 0xfff);
        ra = rand() & 0xff;
        rx = rand() & 0xff;
        ry = rand() & 0xff;

        eff_addr = base + ry;
        predicted_after = ra & rx & ((eff_addr >> 8) + 1);

        before = PEEK(eff_addr);
        sha_absy(base, ra, rx, ry);
        after = PEEK(eff_addr);

        ok = (after == predicted_after);

        printf("%04x  %02x/%02x/%02x | %04x %02x -> %02x (%02x) %s\n", base, ra, rx, ry, eff_addr, before, after, predicted_after, ok ? "ok" : "BAD");

        if (!ok)
        {
            ++count_bad;
        }
    }

    return 0;
}
