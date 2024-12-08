
/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
//                                      adc_and_sbc.h                                      //
//                                                                                         //
//  Hardware-verified implementations of the 6502 and 65C02 "ADC" and "SBC" instructions.  //
//                                                                                         //
//  Implemented in Dec 2024 by Sidney Cadot. Use this code in whatever way you like.       //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ADC_AND_SBC_H
#define ADC_AND_SBC_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t  Accumulator;
    bool     FlagN;
    bool     FlagV;
    bool     FlagZ;
    bool     FlagC;
} AddSubResult;

// 6502 versions of ADC and SBC.
// Verified to produce bitwise identical results to a hardware 6502 for all inputs.
// Verification was done using an Atari 800XL, which has a SALLY 6502 chip.

AddSubResult adc_6502(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand);
AddSubResult sbc_6502(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand);

// 65C02 versions of ADC and SBC.
// Verified to produce bitwise identical results to a hardware 65C02 for all inputs.
// Verification was done using a Neo6502 board, which has a WDC 65C02 chip.

AddSubResult adc_65c02(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand);
AddSubResult sbc_65c02(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand);

#endif
