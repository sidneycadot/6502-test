
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <peekpoke.h>

// Test function for ADC/SBC in 6502/65C02 assembly.
void testfunc(void);

// Interface with 'testfunc'.
extern uint8_t A_VALUE; 
extern uint8_t P_VALUE;
extern uint8_t TABLE[1024];

int main(void)
{
    unsigned a, p;
    //FILE * f;

    //f = fopen("H:adcsbc.dat", "wb");
    //assert(f != NULL);

    for (a = 0; a <= 255; ++a)
    for (p = 0; p <= 255; ++p)
    {
        POKE(A_VALUE, a);
        POKE(P_VALUE, p);
        printf("%u %u\n", p, a);
        testfunc();
        //fwrite(TABLE, 1, 1024, f);
    }
    //fclose(f);
    printf("bye.\n");
    getchar();
    return 0;
}
