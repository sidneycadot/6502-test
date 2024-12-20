
/////////////////////
// test_sha_absy.c //
/////////////////////

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <peekpoke.h>

uint8_t __fastcall__ sha_absy(uint16_t base, uint8_t ra, uint8_t rx, uint8_t ry);

#define DEFAULT_BYTE_VALUE 0xff
#define MALLOC_SIZE 0x7000

int main(void)
{
    char * ptr;
    uint16_t ptru;
    uint16_t base;
    uint16_t eff_addr;
    uint8_t ra, rx, ry;
    uint8_t before, after, predicted_after;
    unsigned long count, count_bad;
    bool ok;
    uint16_t k;

    ptr = malloc(MALLOC_SIZE);
    assert(ptr != NULL);
    printf("ptr: %p\n", ptr);

    memset(ptr, DEFAULT_BYTE_VALUE, MALLOC_SIZE);

    ptru = (uint16_t)ptr;
    while ((ptru & 0xff) != 0)
    {
        ++ptru;
    }

    printf("ptru: %04x\n", ptru);

    for (count = count_bad=0;;++count)
    {
        if (count % 100 == 0)
        {
            printf("count: %lu bad: %lu\n", count, count_bad);
        }

        base = ptru + (rand() & 0x3fff);
        ra = rand();
        rx = rand();
        ry = rand();

        eff_addr = base + ry;

        // This is the behavior predicted by the "No More Secrets" document.
        predicted_after = ra & rx & ((eff_addr >> 8) + 1);

        // Sample effective address before the SHA abs,y; perform the SHA abs,y;
        // re-sample effective address after the SHA abs,y.

        before = PEEK(eff_addr);
        sha_absy(base, ra, rx, ry);
        after = PEEK(eff_addr);

        ok = (after == predicted_after);

        if (ok)
        {
            // The effective address changed as predicted.
            // Restore it to the default byte value.
            POKE(eff_addr, DEFAULT_BYTE_VALUE);
        }
        if (!ok)
        {
            ++count_bad;
            printf("*** ERROR *** (count = %lu):\n", count);
            printf("  testcase: base=%04x ra/rx/ry=%02x/%02x/%02x\n", base, ra, rx, ry);
            printf("    eff_addr: address=%04x\n", eff_addr);
            printf("    expected: after=%02x\n", predicted_after);
            printf("    observed: before=%02x, after=%02x\n", before, after);
            printf("  checking memory ...\n");
            for (k = 0; k < MALLOC_SIZE; ++k)
            {
                if (ptr[k] != DEFAULT_BYTE_VALUE)
                {
                    printf("  found unexpected byte: %p -> 0x%02u\n", &ptr[k], ptr[k]);
                    ptr[k] = DEFAULT_BYTE_VALUE;
                }
            }
            printf("Press enter to continue.\n");
            getchar();
        }
    }

    return 0;
}
