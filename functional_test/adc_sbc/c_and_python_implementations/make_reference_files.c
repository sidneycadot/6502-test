
////////////////////////////
// make_reference_files.c //
////////////////////////////

// This progrem generates the same 6502 and 65C02 test reference files that can also be generated on
// real hardware. This is possible since our re-implementations of the ADC and SBC routines for the
// 6502 and 65C02 are functionally indistinguishable from their hardware counterparts.

#include <stdio.h>
#include <assert.h>

#include "6502_adc_sbc.h"

static void write_result(FILE * fo, AddSubResult * result, unsigned decimal_flag)
{
    // For the status register, assume that the processor's I (Interrupt Disable) flag is zero.
    const uint8_t status_register = (result->FlagN << 7) | (result->FlagV << 6) | (1 << 5) | (1 << 4) | (decimal_flag << 3) | (0 << 2) | (result->FlagZ << 1) | (result->FlagC << 0);
    fputc(result->Accumulator, fo);
    fputc(status_register, fo);

}

typedef AddSubResult testfunc(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand);

static void make_reference_file(const char * filename, testfunc adc, testfunc sbc)
{
    FILE * fo = fopen(filename, "wb");
    assert(fo != NULL);

    printf("Writing hardware behavior reference file: %s ...\n", filename);

    for (unsigned decimal_flag = 0; decimal_flag <= 1; ++decimal_flag)
    {
        for (unsigned initial_carry_flag = 0; initial_carry_flag <= 1; ++initial_carry_flag)
        {
            for (unsigned initial_accumulator = 0; initial_accumulator <= 255; ++initial_accumulator)
            {
                for (unsigned operand = 0; operand <= 255; ++operand)
                {
                    AddSubResult adc_result = adc(decimal_flag, initial_carry_flag, initial_accumulator, operand);
                    AddSubResult sbc_result = sbc(decimal_flag, initial_carry_flag, initial_accumulator, operand);

                    write_result(fo, &adc_result, decimal_flag);
                    write_result(fo, &sbc_result, decimal_flag);
                }
            }
        }
   }
   fclose(fo);
}

int main(void)
{
    make_reference_file("adc_sbc_6502.dat", adc_6502, sbc_6502);
    make_reference_file("adc_sbc_65c02.dat", adc_65c02, sbc_65c02);

    printf("*** IMPORTANT *** make sure to check the generated files against the known-good MD5SUM file !!!\n");

    return 0;
}