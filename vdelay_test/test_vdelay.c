
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define VDELAY_MIN_CYCLES 29u

// Time the execution of VDELAY for the given number of cycles.
uint16_t __fastcall__ time_vdelay(uint16_t cycles);

int main(void)
{
    uint16_t cycles_requested, cycles_actual;
    bool ok;

    // Loop 'cycles_requested' over all values 0 .. 65535.

    printf("\nTesting the VDELAY routine from 0 to 65535 clock cycles ...\n\n");

    ok = true;
    cycles_requested = 0;
    do {
        cycles_actual = time_vdelay(cycles_requested);
        if (cycles_actual != cycles_requested)
        {
            printf("Cycle count mismatch: requested %u, actual %u.\n", cycles_requested, cycles_actual);
            if (cycles_requested >= VDELAY_MIN_CYCLES)
            {
                // Bad cycle count mismatch: VDELAY should work for this cycle count.
                ok = false;
            }
        }
    } while (++cycles_requested != 0);

    // Report result.

    printf("\nTest result: %s.\n\n", ok ? "SUCCESS" : "FAILED");

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
