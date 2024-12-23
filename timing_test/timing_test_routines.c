
////////////////////////////
// timing_test_routines.c //
////////////////////////////

#include "timing_test_memory.h"
#include "timing_test_measurement.h"
#include "timing_test_routines.h"
#include "target.h"

unsigned STEP_SIZE = 85;
unsigned LAST = 255;

#define DEFAULT_RUN_FLAGS (F_STOP_ON_ERROR)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                              TRIVIAL SUPPORT ROUTINES                                             //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t lsb(uint8_t * ptr)
{
    return (uint16_t)ptr & 0xff;
}

static uint8_t msb(uint8_t * ptr)
{
    return (uint16_t)ptr >> 8;
}

static bool different_pages(uint8_t * u1, uint8_t * u2)
{
    return msb(u1) != msb(u2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                          PREPROCESSOR SYMBOLS FOR 6502 OPCODES                                    //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OPC_BRK        0x00
#define OPC_PHP        0x08
#define OPC_ORA_IMM    0x09
#define OPC_JSR_ABS    0x20
#define OPC_PLP        0x28
#define OPC_AND_IMM    0x29
#define OPC_RTI        0x40
#define OPC_PHA        0x48
#define OPC_JMP_ABS    0x4c
#define OPC_RTS        0x60
#define OPC_PLA        0x68
#define OPC_JMP_IND    0x6c
#define OPC_JMP_IND_X  0x7c
#define OPC_STY_ZP     0x84
#define OPC_STA_ZP     0x85
#define OPC_STX_ZP     0x86
#define OPC_STX_ABS    0x8e
#define OPC_TXS        0x9a
#define OPC_LDY_IMM    0xa0
#define OPC_LDX_IMM    0xa2
#define OPC_LDA_IMM    0xa9
#define OPC_LDX_ABS    0xae
#define OPC_TSX        0xba

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                           TIMING TESTS FOR SINGLE, DOUBLE, AND TRIPLE BYTE INSTRUCTION SEQUENCES                  //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_single_byte_instruction_sequence(const char * opcode_description, uint8_t opc, unsigned instruction_cycles)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par1_OpcodeOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    m_test_overhead_cycles = 0;
    m_instruction_cycles = instruction_cycles;

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0] = opc;     // OPC
        opcode_address[1] = OPC_RTS; // RTS [-]

        if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_two_byte_instruction_sequence(const char * opcode_description, uint8_t test_opcode_offset, uint8_t opc1, uint8_t opc2, unsigned test_overhead_cycles, unsigned instruction_cycles)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par1_OpcodeOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    m_test_overhead_cycles = test_overhead_cycles;
    m_instruction_cycles   = instruction_cycles;

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0 - (int)test_opcode_offset] = opc1;    // OPC
        opcode_address[1 - (int)test_opcode_offset] = opc2;    // OPC
        opcode_address[2 - (int)test_opcode_offset] = OPC_RTS; // RTS [-]

        if (!execute_single_opcode_test(opcode_address - test_opcode_offset, DEFAULT_RUN_FLAGS))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_three_byte_instruction_sequence(const char * opcode_description, uint8_t test_opcode_offset, uint8_t opc1, uint8_t opc2, uint8_t opc3, unsigned test_overhead_cycles, unsigned instruction_cycles)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par1_OpcodeOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    m_test_overhead_cycles = test_overhead_cycles;
    m_instruction_cycles   = instruction_cycles;

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0 - (int)test_opcode_offset] = opc1;    // OPC
        opcode_address[1 - (int)test_opcode_offset] = opc2;    // OPC
        opcode_address[2 - (int)test_opcode_offset] = opc3;    // OPC
        opcode_address[3 - (int)test_opcode_offset] = OPC_RTS; // RTS [-]

        if (!execute_single_opcode_test(opcode_address - test_opcode_offset, DEFAULT_RUN_FLAGS))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                         TIMING TESTS FOR READ INSTRUCTIONS                                        //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_read_immediate_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_Immediate);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            opcode_address[0] = opcode;  // OPC #par2 [2]
            opcode_address[1] = par2;    //
            opcode_address[2] = OPC_RTS; // RTS       [-]

            m_test_overhead_cycles = 0;
            m_instruction_cycles = 2;

            if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_ZPage);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            if (zp_address_is_safe_for_read(par2))
            {
                opcode_address[0] = opcode;     // OPC par2        [3]
                opcode_address[1] = par2;       //
                opcode_address[2] = OPC_RTS;    // RTS             [-]

                m_test_overhead_cycles = 0;
                m_instruction_cycles = 3;

                if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                    return false;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_x_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_ZPage_XReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe_for_read(par2 + par3))
                {
                    opcode_address[-2] = OPC_LDX_IMM; // LDX #par3        [2]
                    opcode_address[-1] = par3;        //
                    opcode_address[ 0] = opcode;      // OPC par2,X       [4]
                    opcode_address[ 1] = par2;        //
                    opcode_address[ 2] = OPC_RTS;     // RTS              [-]

                    m_test_overhead_cycles = 2;
                    m_instruction_cycles = 4;

                    if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_y_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_ZPage_YReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe_for_read(par2 + par3))
                {
                    opcode_address[-2] = OPC_LDY_IMM;  // LDY #par3          [2]
                    opcode_address[-1] = par3;         //
                    opcode_address[ 0] = opcode;       // OPC par2,Y         [4]
                    opcode_address[ 1] = par2;         //
                    opcode_address[ 2] = OPC_RTS;      // RTS                [-]

                    m_test_overhead_cycles = 2;
                    m_instruction_cycles = 4;

                    if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_abs_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_AbsOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * read_address = TESTCODE_BASE + par2;

            opcode_address[0] = opcode;            // OPC read_address   [4]
            opcode_address[1] = lsb(read_address); //
            opcode_address[2] = msb(read_address); //
            opcode_address[3] = OPC_RTS;           // RTS                [-]

            m_test_overhead_cycles = 0;
            m_instruction_cycles = 4;

            if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

#if defined(CPU_65C02)
bool timing_test_read_abs_instruction_slow(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_AbsOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * read_address = TESTCODE_BASE + par2;

            opcode_address[0] = opcode;            // OPC read_address   [4]
            opcode_address[1] = lsb(read_address); //
            opcode_address[2] = msb(read_address); //
            opcode_address[3] = OPC_RTS;           // RTS                [-]

            m_test_overhead_cycles = 0;
            m_instruction_cycles = 8; // Unique for instruction 0x5C on the 65C02.

            if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}
#endif

bool timing_test_read_abs_x_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_XReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = OPC_LDX_IMM;       // LDX #par3            [2]
                opcode_address[-1] = par3;              //
                opcode_address[ 0] = opcode;            // OPC base_address,X   [4 or 5]
                opcode_address[ 1] = lsb(base_address); //
                opcode_address[ 2] = msb(base_address); //
                opcode_address[ 3] = OPC_RTS;           // RTS                  [-]

                m_test_overhead_cycles = 2;
                m_instruction_cycles = 4 + different_pages(base_address, base_address + par3);

                if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_abs_y_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_YReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = OPC_LDY_IMM;        // LDY #par3            [2]
                opcode_address[-1] = par3;               //
                opcode_address[ 0] = opcode;             // OPC base_address,Y   [4 or 5]
                opcode_address[ 1] = lsb(base_address);  //
                opcode_address[ 2] = msb(base_address);  //
                opcode_address[ 3] = OPC_RTS;            // RTS                  [-]

                m_test_overhead_cycles = 2;
                m_instruction_cycles = 4 + different_pages(base_address, base_address + par3);

                if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}


bool timing_test_read_abs_y_instruction_save_sp(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_YReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-6] = OPC_TSX;                  // TSX          [2]
                opcode_address[-5] = OPC_STX_ABS;              // STX save_sp  [4]
                opcode_address[-4] = lsb(opcode_address + 8);  //
                opcode_address[-3] = msb(opcode_address + 8);  //
                opcode_address[-2] = OPC_LDY_IMM;              // LDY #par3     2]
                opcode_address[-1] = par3;                     //
                opcode_address[ 0] = opcode;                   // OPC base_address,Y   [4 or 5]
                opcode_address[ 1] = lsb(base_address);        //
                opcode_address[ 2] = msb(base_address);        //
                opcode_address[ 3] = OPC_LDX_ABS;              // LDX save_sp  [4]
                opcode_address[ 4] = lsb(opcode_address + 8);  //
                opcode_address[ 5] = msb(opcode_address + 8);  //
                opcode_address[ 6] = OPC_TXS;                  // TXS          [2]
                opcode_address[ 7] = OPC_RTS;                  // RTS          [-]

                m_test_overhead_cycles = 2 + 4 + 2 + 4 + 2;
                m_instruction_cycles = 4 + different_pages(base_address, base_address + par3);

                if (!execute_single_opcode_test(opcode_address - 6, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_x_indirect_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t   zp_ptr_lo;
    uint8_t   zp_ptr_hi;
    uint8_t * abs_address;

    prepare_opcode_tests(opcode_description, Par1234_OpcodeOffset_ZPage_XReg_AbsOffset);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                // (zp_ptr_lo, zp_ptr_hi) will be the actual pointer used for indirection.
                zp_ptr_lo = par2 + par3;
                zp_ptr_hi = zp_ptr_lo + 1;

                if (zp_address_is_safe_for_write(zp_ptr_lo) && zp_address_is_safe_for_write(zp_ptr_hi))
                {
                    zpage_preserve[0] = zp_ptr_lo;
                    zpage_preserve[1] = zp_ptr_hi;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        abs_address = TESTCODE_BASE + par4;

                        opcode_address[-10] = OPC_LDX_IMM;       // LDX #<abs_address     [2]
                        opcode_address[ -9] = lsb(abs_address);  //
                        opcode_address[ -8] = OPC_STX_ZP;        // STX zp_ptr_lo         [3]
                        opcode_address[ -7] = zp_ptr_lo;         //
                        opcode_address[ -6] = OPC_LDX_IMM;       // LDX #>abs_address     [2]
                        opcode_address[ -5] = msb(abs_address);  //
                        opcode_address[ -4] = OPC_STX_ZP;        // STX zp_ptr_lo         [3]
                        opcode_address[ -3] = zp_ptr_lo;         //
                        opcode_address[ -2] = OPC_LDX_IMM;       // LDX #par3             [2]
                        opcode_address[ -1] = par3;              //
                        opcode_address[  0] = opcode;            // OPC (zpage, X)        [6]
                        opcode_address[  1] = par2;              //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        m_test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        m_instruction_cycles = 6;

                        if (!execute_single_opcode_test(opcode_address - 10, DEFAULT_RUN_FLAGS))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_indirect_y_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t   zp_ptr_lo;
    uint8_t   zp_ptr_hi;
    uint8_t * base_address;

    prepare_opcode_tests(opcode_description, Par1234_OpcodeOffset_ZPage_AbsOffset_YReg);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            zp_ptr_lo = par2;
            zp_ptr_hi = zp_ptr_lo + 1;

            if (zp_address_is_safe_for_write(zp_ptr_lo) && zp_address_is_safe_for_write(zp_ptr_hi))
            {
                zpage_preserve[0] = zp_ptr_lo;
                zpage_preserve[1] = zp_ptr_hi;

                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    base_address = TESTCODE_BASE + par3;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        opcode_address[-10] = OPC_LDY_IMM;        // LDY #<base_address    [2]
                        opcode_address[ -9] = lsb(base_address);  //
                        opcode_address[ -8] = OPC_STY_ZP;         // STY zp_ptr_lo         [3]
                        opcode_address[ -7] = zp_ptr_lo;          //
                        opcode_address[ -6] = OPC_LDY_IMM;        // LDY #>base_address    [2]
                        opcode_address[ -5] = msb(base_address);  //
                        opcode_address[ -4] = OPC_STY_ZP;         // STY zp_ptr_hi         [3]
                        opcode_address[ -3] = zp_ptr_hi;          //
                        opcode_address[ -2] = OPC_LDY_IMM;        // LDY #par4             [2]
                        opcode_address[ -1] = par4;               //
                        opcode_address[  0] = opcode;             // OPC (zpage), Y        [5 or 6]
                        opcode_address[  1] = zp_ptr_lo;          //
                        opcode_address[  2] = OPC_RTS;            // RTS                   [-]

                        m_test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        m_instruction_cycles = 5 + different_pages(base_address, base_address + par4);

                        if (!execute_single_opcode_test(opcode_address - 10, DEFAULT_RUN_FLAGS))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                    if (par3 == LAST)
                        break;
                }
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

#if defined(CPU_65C02)
bool timing_test_read_zpage_indirect_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t   zp_ptr_lo;
    uint8_t   zp_ptr_hi;
    uint8_t * effective_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_ZPage_AbsOffset);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            zp_ptr_lo = par2;
            zp_ptr_hi = zp_ptr_lo + 1;

            if (zp_address_is_safe_for_write(zp_ptr_lo) && zp_address_is_safe_for_write(zp_ptr_hi))
            {
                zpage_preserve[0] = zp_ptr_lo;
                zpage_preserve[1] = zp_ptr_hi;

                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    effective_address = TESTCODE_BASE + par3;

                    opcode_address[-8] = OPC_LDA_IMM;             // LDA #<base_address    [2]
                    opcode_address[-7] = lsb(effective_address);  //
                    opcode_address[-6] = OPC_STA_ZP;              // STA zp_ptr_lo         [3]
                    opcode_address[-5] = zp_ptr_lo;               //
                    opcode_address[-4] = OPC_LDA_IMM;             // LDA #>base_address    [2]
                    opcode_address[-3] = msb(effective_address);  //
                    opcode_address[-2] = OPC_STA_ZP;              // STA zp_ptr_hi         [3]
                    opcode_address[-1] = zp_ptr_hi;               //
                    opcode_address[ 0] = opcode;                  // OPC (zpage)           [5]
                    opcode_address[ 1] = zp_ptr_lo;               //
                    opcode_address[ 2] = OPC_RTS;                 // RTS                   [-]

                    m_test_overhead_cycles = 2 + 3 + 2 + 3;
                    m_instruction_cycles = 5;

                    if (!execute_single_opcode_test(opcode_address - 8, DEFAULT_RUN_FLAGS))
                        return false;

                    if (par3 == LAST)
                        break;
                }
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                        TIMING TESTS FOR WRITE INSTRUCTIONS                                        //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_write_zpage_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_ZPage);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            if (zp_address_is_safe_for_write(par2))
            {
                zpage_preserve[0] = par2;

                opcode_address[0] = opcode;  // OPC par2   [3]
                opcode_address[1] = par2;    //
                opcode_address[2] = OPC_RTS; // RTS        [-]

                m_test_overhead_cycles = 0;
                m_instruction_cycles = 3;

                if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                    return false;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_zpage_x_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_ZPage_XReg);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe_for_write(par2 + par3))
                {
                    zpage_preserve[0] = par2 + par3;

                    opcode_address[-2] = OPC_LDX_IMM;  // LDX #imm       [2]
                    opcode_address[-1] = par3;         //
                    opcode_address[ 0] = opcode;       // OPC par2,X     [4]
                    opcode_address[ 1] = par2;         //
                    opcode_address[ 2] = OPC_RTS;      // RTS            [1]

                    m_test_overhead_cycles = 2;
                    m_instruction_cycles = 4;

                    if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_zpage_y_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_ZPage_YReg);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe_for_write(par2 + par3))
                {
                    zpage_preserve[0] = par2 + par3;

                    opcode_address[-2] = OPC_LDY_IMM;  // LDY #imm    [2]
                    opcode_address[-1] = par3;         //
                    opcode_address[ 0] = opcode;       // OPC par2,Y  [4]
                    opcode_address[ 1] = par2;         //
                    opcode_address[ 2] = OPC_RTS;      // RTS         [-]

                    m_test_overhead_cycles = 2;
                    m_instruction_cycles = 4;

                    if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_abs_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t * write_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_AbsOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            write_address = TESTCODE_BASE + par2;

            opcode_address[0] = opcode;              // OPC write_address   [4]
            opcode_address[1] = lsb(write_address);  //
            opcode_address[2] = msb(write_address);  //
            opcode_address[3] = OPC_RTS;             // RTS                 [-]

            m_test_overhead_cycles = 0;
            m_instruction_cycles = 4;

            if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_abs_x_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t * base_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_XReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = OPC_LDX_IMM;        // LDX #par3            [2]
                opcode_address[-1] = par3;               //
                opcode_address[ 0] = opcode;             // OPC base_address,X   [5]
                opcode_address[ 1] = lsb(base_address);  //
                opcode_address[ 2] = msb(base_address);  //
                opcode_address[ 3] = OPC_RTS;            // RTS                  [-]

                m_test_overhead_cycles = 2;
                m_instruction_cycles = 5;

                if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_abs_y_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t * base_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_YReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = OPC_LDY_IMM;        // LDY #par3            [2]
                opcode_address[-1] = par3;               //
                opcode_address[ 0] = opcode;             // OPC base_address,Y   [5]
                opcode_address[ 1] = lsb(base_address);  //
                opcode_address[ 2] = msb(base_address);  //
                opcode_address[ 3] = OPC_RTS;            // RTS                  [-]

                m_test_overhead_cycles = 2;
                m_instruction_cycles = 5;

                if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_abs_y_instruction_save_sp(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t * base_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_YReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-6] = OPC_TSX;                  // TSX          [2]
                opcode_address[-5] = OPC_STX_ABS;              // STX save_sp  [4]
                opcode_address[-4] = lsb(opcode_address + 8);  //
                opcode_address[-3] = msb(opcode_address + 8);  //
                opcode_address[-2] = OPC_LDY_IMM;              // LDY #imm     [2]
                opcode_address[-1] = par3;                     //
                opcode_address[ 0] = opcode;                   // OPC base_address,Y   [5]
                opcode_address[ 1] = lsb(base_address);        //
                opcode_address[ 2] = msb(base_address);        //
                opcode_address[ 3] = OPC_LDX_ABS;              // LDX save_sp  [4]
                opcode_address[ 4] = lsb(opcode_address + 8);  //
                opcode_address[ 5] = msb(opcode_address + 8);  //
                opcode_address[ 6] = OPC_TXS;                  // TXS          [2]
                opcode_address[ 7] = OPC_RTS;                  // RTS          [-]

                m_test_overhead_cycles = 2 + 4 + 2 + 4 + 2;
                m_instruction_cycles = 5;

                if (!execute_single_opcode_test(opcode_address - 6, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_zpage_x_indirect_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t   zp_ptr_lo;
    uint8_t   zp_ptr_hi;
    uint8_t * abs_address;

    prepare_opcode_tests(opcode_description, Par1234_OpcodeOffset_ZPage_XReg_AbsOffset);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                // (zp_ptr_lo, zp_ptr_hi) will be the actual pointer used for indirection.
                zp_ptr_lo = par2 + par3;
                zp_ptr_hi = zp_ptr_lo + 1;

                if (zp_address_is_safe_for_write(zp_ptr_lo) && zp_address_is_safe_for_write(zp_ptr_hi))
                {
                    zpage_preserve[0] = zp_ptr_lo;
                    zpage_preserve[1] = zp_ptr_hi;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        abs_address = TESTCODE_BASE + par4;

                        opcode_address[-10] = OPC_LDX_IMM;       // LDX #<abs_address     [2]
                        opcode_address[ -9] = lsb(abs_address);  //
                        opcode_address[ -8] = OPC_STX_ZP;        // STX zp_ptr_lo [3]
                        opcode_address[ -7] = zp_ptr_lo;         //
                        opcode_address[ -6] = OPC_LDX_IMM;       // LDX #>abs_address     [2]
                        opcode_address[ -5] = msb(abs_address);  //
                        opcode_address[ -4] = OPC_STX_ZP;        // STX zp_ptr_hi [3]
                        opcode_address[ -3] = zp_ptr_hi;         //
                        opcode_address[ -2] = OPC_LDX_IMM;       // LDX #par3             [2]
                        opcode_address[ -1] = par3;              //
                        opcode_address[  0] = opcode;            // OPC (zpage, X)        [6]
                        opcode_address[  1] = par2;              //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        m_test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        m_instruction_cycles = 6;

                        if (!execute_single_opcode_test(opcode_address - 10, DEFAULT_RUN_FLAGS))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_zpage_indirect_y_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t   zp_ptr_lo;
    uint8_t   zp_ptr_hi;

    prepare_opcode_tests(opcode_description, Par1234_OpcodeOffset_ZPage_AbsOffset_YReg);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            zp_ptr_lo = par2;
            zp_ptr_hi = zp_ptr_lo + 1;

            if (zp_address_is_safe_for_write(zp_ptr_lo) && zp_address_is_safe_for_write(zp_ptr_hi))
            {
                zpage_preserve[0] = zp_ptr_lo;
                zpage_preserve[1] = zp_ptr_hi;

                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    uint8_t * base_address = TESTCODE_BASE + par3;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        opcode_address[-10] = OPC_LDY_IMM;        // LDY #<base_address    [2]
                        opcode_address[ -9] = lsb(base_address);  //
                        opcode_address[ -8] = OPC_STY_ZP;         // STY zp_ptr_lo [3]
                        opcode_address[ -7] = zp_ptr_lo;          //
                        opcode_address[ -6] = OPC_LDY_IMM;        // LDY #>base_address    [2]
                        opcode_address[ -5] = msb(base_address);  //
                        opcode_address[ -4] = OPC_STY_ZP;         // STY zp_ptr_hi         [3]
                        opcode_address[ -3] = zp_ptr_hi;          //
                        opcode_address[ -2] = OPC_LDY_IMM;        // LDY #par4             [2]
                        opcode_address[ -1] = par4;               //
                        opcode_address[  0] = opcode;             // OPC (zpage), Y        [5 or 6]
                        opcode_address[  1] = zp_ptr_lo;          //
                        opcode_address[  2] = OPC_RTS;            // RTS                   [-]

                        m_test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        m_instruction_cycles = 6;

                        if (!execute_single_opcode_test(opcode_address - 10, DEFAULT_RUN_FLAGS))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                    if (par3 == LAST)
                        break;
                }
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

#if defined(CPU_65C02)
bool timing_test_write_zpage_indirect_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t   zp_ptr_lo;
    uint8_t   zp_ptr_hi;
    uint8_t * effective_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_ZPage_AbsOffset);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            zp_ptr_lo = par2;
            zp_ptr_hi = zp_ptr_lo + 1;

            if (zp_address_is_safe_for_write(zp_ptr_lo) && zp_address_is_safe_for_write(zp_ptr_hi))
            {
                zpage_preserve[0] = zp_ptr_lo;
                zpage_preserve[1] = zp_ptr_hi;

                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    effective_address = TESTCODE_BASE + par3;

                    opcode_address[-8] = OPC_LDA_IMM;             // LDA #<base_address    [2]
                    opcode_address[-7] = lsb(effective_address);  //
                    opcode_address[-6] = OPC_STA_ZP;              // STA zp_ptr_lo         [3]
                    opcode_address[-5] = zp_ptr_lo;               //
                    opcode_address[-4] = OPC_LDA_IMM;             // LDA #>base_address    [2]
                    opcode_address[-3] = msb(effective_address);  //
                    opcode_address[-2] = OPC_STA_ZP;              // STA zp_ptr_hi         [3]
                    opcode_address[-1] = zp_ptr_hi;               //
                    opcode_address[ 0] = opcode;                  // OPC (zpage)           [5]
                    opcode_address[ 1] = zp_ptr_lo;               //
                    opcode_address[ 2] = OPC_RTS;                 // RTS                   [-]

                    m_test_overhead_cycles = 2 + 3 + 2 + 3;
                    m_instruction_cycles = 5;

                    if (!execute_single_opcode_test(opcode_address - 8, DEFAULT_RUN_FLAGS))
                        return false;

                    if (par3 == LAST)
                        break;
                }
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                    TIMING TESTS FOR READ-MODIFY-WRITE INSTRUCTIONS                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_read_modify_write_zpage_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_ZPage);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            if (zp_address_is_safe_for_write(par2))
            {
                zpage_preserve[0] = par2;

                opcode_address[0] = opcode;   // OPC par2    [5]
                opcode_address[1] = par2;     //
                opcode_address[2] = OPC_RTS;  // RTS         [-]

                m_test_overhead_cycles = 0;
                m_instruction_cycles = 5;

                if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                    return false;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_zpage_x_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_ZPage_XReg);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe_for_write(par2 + par3))
                {
                    zpage_preserve[0] = par2 + par3;

                    opcode_address[-2] = OPC_LDX_IMM;  // LDX #par3     [2]
                    opcode_address[-1] = par3;         //
                    opcode_address[ 0] = opcode;       // OPC par2,X    [6]
                    opcode_address[ 1] = par2;         //
                    opcode_address[ 2] = OPC_RTS;      // RTS           [-]

                    m_test_overhead_cycles = 2;
                    m_instruction_cycles = 6;

                    if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_abs_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t * abs_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_AbsOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            abs_address = TESTCODE_BASE + par2;

            opcode_address[0] = opcode;            // OPC abs_address    [6]
            opcode_address[1] = lsb(abs_address);  //
            opcode_address[2] = msb(abs_address);  //
            opcode_address[3] = OPC_RTS;           // RTS                [-]

            m_test_overhead_cycles = 0;
            m_instruction_cycles = 6;

            if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_abs_x_instruction_v1(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t * base_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_XReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = OPC_LDX_IMM;        // LDX #par3            [2]
                opcode_address[-1] = par3;               //
                opcode_address[ 0] = opcode;             // OPC base_address,X   [7]
                opcode_address[ 1] = lsb(base_address);  //
                opcode_address[ 2] = msb(base_address);  //
                opcode_address[ 3] = OPC_RTS;            // RTS                  [-]

                m_test_overhead_cycles = 2;
                m_instruction_cycles = 7;

                if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

#if defined(CPU_65C02)
// Note: this code is only ever useful on a 65C02 processor.
bool timing_test_read_modify_write_abs_x_instruction_v2(const char * opcode_description, uint8_t opcode)
{
    // The ASL/ROL/ROR/LSR instructions with absolute-x addressing have different timing on the 65C02 compared to the 6502.
    // Note: the DEC/INC with absolute-x indexing on the 65C02 have the same timing as their 6502 counterparts. Go figure.

    uint8_t * opcode_address;
    uint8_t * base_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_XReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = OPC_LDX_IMM;        // LDX #par3            [2]
                opcode_address[-1] = par3;               //
                opcode_address[ 0] = opcode;             // OPC base_address,X   [7]
                opcode_address[ 1] = lsb(base_address);  //
                opcode_address[ 2] = msb(base_address);  //
                opcode_address[ 3] = OPC_RTS;            // RTS                  [-]

                m_test_overhead_cycles = 2;
                m_instruction_cycles = 6 + different_pages(base_address, base_address + par3);

                if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}
#endif

bool timing_test_read_modify_write_abs_y_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t * base_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_YReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = OPC_LDY_IMM;        // LDY #par3            [2]
                opcode_address[-1] = par3;               //
                opcode_address[ 0] = opcode;             // OPC base_address,Y   [7]
                opcode_address[ 1] = lsb(base_address);  //
                opcode_address[ 2] = msb(base_address);  //
                opcode_address[ 3] = OPC_RTS;            // RTS                  [-]

                m_test_overhead_cycles = 2;
                m_instruction_cycles = 7;

                if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_zpage_x_indirect_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t   zp_ptr_lo;
    uint8_t   zp_ptr_hi;
    uint8_t * abs_address;

    prepare_opcode_tests(opcode_description, Par1234_OpcodeOffset_ZPage_XReg_AbsOffset);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                // (zp_ptr_lo, zp_ptr_hi) will be the actual pointer used for indirection.
                zp_ptr_lo = par2 + par3;
                zp_ptr_hi = zp_ptr_lo + 1;

                zpage_preserve[0] = zp_ptr_lo;
                zpage_preserve[1] = zp_ptr_hi;

                if (zp_address_is_safe_for_write(zp_ptr_lo) && zp_address_is_safe_for_write(zp_ptr_hi))
                {
                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        abs_address = TESTCODE_BASE + par4;

                        opcode_address[-10] = OPC_LDX_IMM;       // LDX #<abs_address     [2]
                        opcode_address[ -9] = lsb(abs_address);  //
                        opcode_address[ -8] = OPC_STX_ZP;        // STX zp_ptr_lo [3]
                        opcode_address[ -7] = zp_ptr_lo;         //
                        opcode_address[ -6] = OPC_LDX_IMM;       // LDX #>abs_address     [2]
                        opcode_address[ -5] = msb(abs_address);  //
                        opcode_address[ -4] = OPC_STX_ZP;        // STX zp_ptr_hi [3]
                        opcode_address[ -3] = zp_ptr_hi;         //
                        opcode_address[ -2] = OPC_LDX_IMM;       // LDX #par3             [2]
                        opcode_address[ -1] = par3;              //
                        opcode_address[  0] = opcode;            // OPC (zpage, X)        [6]
                        opcode_address[  1] = par2;              //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        m_test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        m_instruction_cycles = 8;

                        if (!execute_single_opcode_test(opcode_address - 10, DEFAULT_RUN_FLAGS))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_zpage_indirect_y_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t   zp_ptr_lo;
    uint8_t   zp_ptr_hi;
    uint8_t * base_address;

    prepare_opcode_tests(opcode_description, Par1234_OpcodeOffset_ZPage_AbsOffset_YReg);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            zp_ptr_lo = par2;
            zp_ptr_hi = zp_ptr_lo + 1;

            if (zp_address_is_safe_for_write(zp_ptr_lo) && zp_address_is_safe_for_write(zp_ptr_hi))
            {
                zpage_preserve[0] = zp_ptr_lo;
                zpage_preserve[1] = zp_ptr_hi;

                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    base_address = TESTCODE_BASE + par3;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        opcode_address[-10] = OPC_LDY_IMM;        // LDY #<base_address    [2]
                        opcode_address[ -9] = lsb(base_address);  //
                        opcode_address[ -8] = OPC_STY_ZP;         // STY zp_ptr_lo [3]
                        opcode_address[ -7] = zp_ptr_lo;          //
                        opcode_address[ -6] = OPC_LDY_IMM;        // LDY #>base_address    [2]
                        opcode_address[ -5] = msb(base_address);  //
                        opcode_address[ -4] = OPC_STY_ZP;         // STY zp_ptr_hi [3]
                        opcode_address[ -3] = zp_ptr_hi;          //
                        opcode_address[ -2] = OPC_LDY_IMM;        // LDY #reg_y            [2]
                        opcode_address[ -1] = par4;               //
                        opcode_address[  0] = opcode;             // OPC (zpage), Y        [5 or 6]
                        opcode_address[  1] = zp_ptr_lo;          //
                        opcode_address[  2] = OPC_RTS;            // RTS                   [-]

                        m_test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        m_instruction_cycles = 8;

                        if (!execute_single_opcode_test(opcode_address - 10, DEFAULT_RUN_FLAGS))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                    if (par3 == LAST)
                        break;
                }
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                            TIMING TESTS FOR BRANCH, JUMP, and INTERRUPT-RELATED INSTRUCTIONS                      //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_branch_instruction(const char * opcode_description, uint8_t opcode, bool branch_when_flag_set)
{
    // This function tests any of the "branch" instructions, testing both the "branch taken" and "branch not taken"
    // scenarios.
    //
    // The 'flag_value' parameter determines whether "branch taken" happens when the associated flag is High or Low.
    // The instructions BMI, BCS, BVS, and BEQ jump when N/C/V/Z are 1; BPL, BCC, BVC and BNE jump when N/C/V/Z are 0.
    //
    // Branch instructions, when not taken, take 2 clock cycles. When taken, they take 3 or 4 clock cycles:
    //
    // * 3 clock cycles if the address following the branch instruction is on the same memory page as the destination address;
    // * 4 clock cycles if the address following the branch instruction is *not* on the same memory page as the destination address.

    // This function tests any of the "branch" instructions, assuming the flag associated with the instruction
    // is in a state that lead to the branch *NOT* being taken.
    //
    // The flag value (true or false) that leads to the branch being not taken is passed in the 'flag_value' parameter.
    // Before executing the branch instructions, the CPU flags that can be used for branching (N, V, Z, C) are all
    // set to this value.
    //
    // Branch instructions, when not taken, always take 2 clock cycles.

    uint8_t * opcode_address;
    uint8_t * entry_address  = TESTCODE_BASE;
    int       displacement;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_BranchDisplacement_TakenNotTaken);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            // Branch Not Taken measurement.

            par3 = 0;

            // If 'branch_when_flag_set' is true, set the N/V/C/Z flags all to zero.
            // If 'branch_when_flag_set' is false, set the N/V/C/Z flags all to one.

            opcode_address[-6] = OPC_PHP;                                           // PHP                  [3]
            opcode_address[-5] = OPC_PLA;                                           // PLA                  [4]
            opcode_address[-4] = branch_when_flag_set ? OPC_AND_IMM : OPC_ORA_IMM;  // AND #$3C / ORA #$C3  [2]
            opcode_address[-3] = branch_when_flag_set ?     0x3c    :     0xc3;     //
            opcode_address[-2] = OPC_PHA;                                           // PHA                  [3]
            opcode_address[-1] = OPC_PLP;                                           // PLP                  [4]
            opcode_address[ 0] = opcode;                                            // Bxx operand          [2]
            opcode_address[ 1] = par2;                                              //
            opcode_address[ 2] = OPC_RTS;                                           // RTS                  [-]

            m_test_overhead_cycles = 3 + 4 + 2 + 3 + 4;
            m_instruction_cycles = 2;

            if (!execute_single_opcode_test(opcode_address - 6, DEFAULT_RUN_FLAGS))
                return false;

            // Branch Taken measurement.
            //
            // For a Branch Taken instruction, the target address should hold an RTS, bit
            // this RTS cannot overlap with the branch opcode itself, or its operand (the displacement).
            //
            // So the par2 values 0xff (displacement -1) and 0xfe (displacement - 2) must be skipped.
            //
            // To maximize the range that we can cover, we jump to the test code from an 'entry point'
            // region.
            if ((par2 & 0xfe) != 0xfe)
            {
                displacement = (par2 <= 0x7f) ? par2 : par2 - 0x100;

                par3 = 1;

                // If 'branch_when_flag_set' is true,set the N/V/C/Z flags all to one.
                // If 'branch_when_flag_set' is false, set the N/V/C/Z flags all to zero.

                entry_address[0] = OPC_PHP;                                           // PHP                  [3]
                entry_address[1] = OPC_PLA;                                           // PLA                  [4]
                entry_address[2] = branch_when_flag_set ? OPC_ORA_IMM : OPC_AND_IMM;  // ORA #$C3 / AND #$3C  [2]
                entry_address[3] = branch_when_flag_set ?     0xc3    :     0x3c;     //
                entry_address[4] = OPC_PHA;                                           // PHA                  [3]
                entry_address[5] = OPC_PLP;                                           // PLP                  [4]
                entry_address[6] = OPC_JMP_ABS;                                       // JMP opcode_address   [3]
                entry_address[7] = lsb(opcode_address);                               //
                entry_address[8] = msb(opcode_address);                               //

                opcode_address[0] = opcode;                                           // Bxx operand          [2]
                opcode_address[1] = par2;                                             //

                opcode_address[2 + displacement] = OPC_RTS;                           // RTS                  [-]

                m_test_overhead_cycles = 3 + 4 + 2 + 3 + 4 + 3;
                m_instruction_cycles = 3 + different_pages(opcode_address + 2, opcode_address + 2 + displacement);

                if (!execute_single_opcode_test(entry_address, DEFAULT_RUN_FLAGS))
                    return false;
            } // displacement acceptable?

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true; // displacement acceptable?
}

#if defined(CPU_65C02)
bool timing_test_bit_branch_instruction(const char * opcode_description, uint8_t opcode, bool branch_when_bit_set)
{
    // This function tests any of the "branch" instructions, testing both the "branch taken" and "branch not taken"
    // scenarios.
    //
    // The 'flag_value' parameter determines whether "branch taken" happens when the associated flag is High or Low.
    // The instructions BMI, BCS, BVS, and BEQ jump when N/C/V/Z are 1; BPL, BCC, BVC and BNE jump when N/C/V/Z are 0.
    //
    // Branch instructions, when not taken, take 2 clock cycles. When taken, they take 3 or 4 clock cycles:
    //
    // * 3 clock cycles if the address following the branch instruction is on the same memory page as the destination address;
    // * 4 clock cycles if the address following the branch instruction is *not* on the same memory page as the destination address.

    // This function tests any of the "branch" instructions, assuming the flag associated with the instruction
    // is in a state that lead to the branch *NOT* being taken.
    //
    // The flag value (true or false) that leads to the branch being not taken is passed in the 'flag_value' parameter.
    // Before executing the branch instructions, the CPU flags that can be used for branching (N, V, Z, C) are all
    // set to this value.
    //
    // Branch instructions, when not taken, always take 2 clock cycles.

    uint8_t * opcode_address;
    uint8_t * entry_address  = TESTCODE_BASE;
    int       displacement;

    prepare_opcode_tests(opcode_description, Par1234_OpcodeOffset_ZPage_BranchDisplacement_TakenNotTaken);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            if (zp_address_is_safe_for_write(par2))
            {
                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    // Branch Not Taken measurement.

                    par4 = 0;

                    // If 'branch_when_bit_set' is true, set the zpage address bits all to zero.
                    // If 'branch_when_bit_set' is false, set the zpage address bits all to one.

                    opcode_address[-4] = OPC_LDA_IMM;                          // LDA #value             [2]
                    opcode_address[-3] = branch_when_bit_set ?   0x00 : 0xff;  //
                    opcode_address[-2] = OPC_STA_ZP;                           // STA par2               [3]
                    opcode_address[-1] = par2;                                 //
                    opcode_address[ 0] = opcode;                               // BBxy zp,distplacement  [5]
                    opcode_address[ 1] = par2;                                 // (zpage)
                    opcode_address[ 2] = par3;                                 // (displacement)         [-]
                    opcode_address[ 3] = OPC_RTS;                              // RTS                    [-]

                    m_test_overhead_cycles = 2 + 3;
                    m_instruction_cycles = 5;

                    if (!execute_single_opcode_test(opcode_address - 4, DEFAULT_RUN_FLAGS))
                        return false;

                    // Branch Taken measurement.
                    //
                    // For a Branch Taken instruction, the target address should hold an RTS, bit
                    // this RTS cannot overlap with the branch opcode itself, or its operand (the displacement).
                    //
                    // So the par2 values 0xff (displacement -1) and 0xfe (displacement - 2) must be skipped.
                    //
                    // To maximize the range that we can cover, we jump to the test code from an 'entry point'
                    // region.
                    if (!(par3 == 0xfd || par3 == 0xfe || par3 == 0xff))
                    {
                        par4 = 1;

                        displacement = (par3 <= 0x7f) ? par3 : par3 - 0x100;

                        // If 'branch_when_bit_set' is true, set the zpage address bits all to one.
                        // If 'branch_when_bit_set' is false, set the zpage address bits all to zero.

                        entry_address[0] = OPC_LDA_IMM;                          // LDA #value             [2]
                        entry_address[1] = branch_when_bit_set ?   0xff : 0x00;  //
                        entry_address[2] = OPC_STA_ZP;                           // STA par2               [3]
                        entry_address[3] = par2;                                 //
                        entry_address[4] = OPC_JMP_ABS;                          // JMP opcode_address     [3]
                        entry_address[5] = lsb(opcode_address);                  //
                        entry_address[6] = msb(opcode_address);                  //

                        opcode_address[0] = opcode;                              // BBxy operand           [6/7]
                        opcode_address[1] = par2;                                //
                        opcode_address[2] = par3;                                //

                        opcode_address[3 + displacement] = OPC_RTS;              // RTS                    [-]

                        m_test_overhead_cycles = 2 + 3 + 3;
                        m_instruction_cycles = 6 + different_pages(opcode_address + 3, opcode_address + 3 + displacement);

                        if (!execute_single_opcode_test(entry_address, DEFAULT_RUN_FLAGS))
                            return false;
                    } // displacement acceptable?

                    if (par3 == LAST)
                        break;
                }
            } // ZP address safe?
            if (par2 == LAST)
                break;
        } // par2 loop
        if (par1 == LAST)
            break;
    } // par1 loop
    return true;
}
#endif

#if defined(CPU_65C02)
bool timing_test_branch_always_instruction(const char * opcode_description, uint8_t opcode)
{
    uint8_t * opcode_address;
    uint8_t * entry_address  = TESTCODE_BASE;
    int       displacement;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_BranchDisplacement_TakenNotTaken);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            if ((par2 & 0xfe) != 0xfe)
            {
                displacement = (par2 <= 0x7f) ? par2 : par2 - 0x100;

                par3 = 1;

                // If 'branch_when_flag_set' is true,set the N/V/C/Z flags all to one.
                // If 'branch_when_flag_set' is false, set the N/V/C/Z flags all to zero.

                entry_address[0] = OPC_JMP_ABS;              // JMP opcode_address   [3]
                entry_address[1] = lsb(opcode_address);      //
                entry_address[2] = msb(opcode_address);      //

                opcode_address[0] = opcode;                  // Bxx operand          [2]
                opcode_address[1] = par2;                    //

                opcode_address[2 + displacement] = OPC_RTS;  // RTS                  [-]

                m_test_overhead_cycles = 3;
                m_instruction_cycles = 3 + different_pages(opcode_address + 2, opcode_address + 2 + displacement);

                if (!execute_single_opcode_test(entry_address, DEFAULT_RUN_FLAGS))
                    return false;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}
#endif

bool timing_test_jmp_abs_instruction(const char * opcode_description)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par1_OpcodeOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0] = OPC_JMP_ABS;              // JMP abs      [3]
        opcode_address[1] = lsb(opcode_address + 3);  //
        opcode_address[2] = msb(opcode_address + 3);  //
        opcode_address[3] = OPC_RTS;                  // RTS          [-]

        m_test_overhead_cycles = 0;
        m_instruction_cycles   = 3;

        if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_jmp_abs_indirect_instruction(const char * opcode_description)
{
    uint8_t * opcode_address;
    uint8_t * target_ptr_address;

    prepare_opcode_tests(opcode_description, Par12_OpcodeOffset_AbsOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            target_ptr_address = TESTCODE_BASE + par2;

#if defined(CPU_6502)

            // The jump-indirect instruction suffers from the "JUMP INDIRECT" bug on
            // the original 6502; this bug was corrected in later variants like the 65C02.
            //
            // On the 6502, if the absolute address low and high bytes are on different pages,
            // the target address would be read from the wrong location. We deal with that here.
            if (different_pages(target_ptr_address + 0, target_ptr_address + 1))
            {
                // Deal with the 6502 bug.
                target_ptr_address[0        ] = lsb(opcode_address + 3);
                target_ptr_address[1 - 0x100] = msb(opcode_address + 3);
            }
            else
            {
                // Normal behavior.
                target_ptr_address[0] = lsb(opcode_address + 3);
                target_ptr_address[1] = msb(opcode_address + 3);
            }

            opcode_address[0] = OPC_JMP_IND;              // JMP (ind)    [5]
            opcode_address[1] = lsb(target_ptr_address);  //
            opcode_address[2] = msb(target_ptr_address);  //
            opcode_address[3] = OPC_RTS;                  // RTS          [-]

            m_test_overhead_cycles = 0;
            m_instruction_cycles   = 5; // The instruction takes 5 cycles on the 6502.

#elif defined(CPU_65C02)

            target_ptr_address[0] = lsb(opcode_address + 3);
            target_ptr_address[1] = msb(opcode_address + 3);

            opcode_address[0] = OPC_JMP_IND;              // JMP (ind)    [6]
            opcode_address[1] = lsb(target_ptr_address);  //
            opcode_address[2] = msb(target_ptr_address);  //
            opcode_address[3] = OPC_RTS;                  // RTS          [-]

            m_test_overhead_cycles = 0;
            m_instruction_cycles   = 6; // The instruction takes 6 cycles on the 65C02.
#else
#error "CPU type not specified."
#endif

            if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

#if defined(CPU_65C02)
bool timing_test_jmp_abs_x_indirect_instruction(const char * opcode_description)
{
    uint8_t * opcode_address;
    uint8_t * target_ptr_address;

    prepare_opcode_tests(opcode_description, Par123_OpcodeOffset_AbsOffset_XReg);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                target_ptr_address = TESTCODE_BASE + par2 + par3;

                target_ptr_address[0] = lsb(opcode_address + 3);
                target_ptr_address[1] = msb(opcode_address + 3);

                opcode_address[-2] = OPC_LDX_IMM;                // LDX #imm     [2]
                opcode_address[-1] = par3;                       //
                opcode_address[ 0] = OPC_JMP_IND_X;              // JMP (ind,X)  [6]
                opcode_address[ 1] = lsb(TESTCODE_BASE + par2);  //
                opcode_address[ 2] = msb(TESTCODE_BASE + par2);  //
                opcode_address[ 3] = OPC_RTS;                    // RTS          [-]

                m_test_overhead_cycles = 2;
                m_instruction_cycles   = 6;

                if (!execute_single_opcode_test(opcode_address - 2, DEFAULT_RUN_FLAGS))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}
#endif

bool timing_test_jsr_abs_instruction(const char * opcode_description)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par1_OpcodeOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0] = OPC_JSR_ABS;              // JSR abs      [6]
        opcode_address[1] = lsb(opcode_address + 3);  //
        opcode_address[2] = msb(opcode_address + 3);  //
        opcode_address[3] = OPC_RTS;                  // RTS          [-]

        m_test_overhead_cycles = 6; // The RTS to get back.
        m_instruction_cycles   = 6;

        if (!execute_single_opcode_test(opcode_address, DEFAULT_RUN_FLAGS))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_rts_instruction(const char * opcode_description)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par1_OpcodeOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        // Note: the RTS is both used as the instruction-under-test, and as the RTS back to
        // the measurement routine.

        opcode_address[-6] = OPC_LDA_IMM;              // LDA #>(rts_address - 1)   [2]
        opcode_address[-5] = msb(opcode_address - 1);  //
        opcode_address[-4] = OPC_PHA;                  // PHA                       [3]
        opcode_address[-3] = OPC_LDA_IMM;              // LDA #<(rts_address - 1)   [2]
        opcode_address[-2] = lsb(opcode_address - 1);  //
        opcode_address[-1] = OPC_PHA;                  // PHA                       [3]
        opcode_address[ 0] = OPC_RTS;                  // RTS                       [6]

        m_test_overhead_cycles = 2 + 3 + 2 + 3;
        m_instruction_cycles   = 6;

        if (!execute_single_opcode_test(opcode_address - 6, DEFAULT_RUN_FLAGS))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_brk_instruction(const char * opcode_description)
{
    uint8_t * opcode_address;
    uint8_t * oldvec;
    bool      proceed;

    prepare_opcode_tests(opcode_description, Par1_OpcodeOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        // We avoid assumptions on what the ISR does before we get back control.
        // It may push to the stack, for example
        // So we restore the stack by direct S-register manipulation.

        opcode_address[-4] = OPC_TSX;                  // TSX          [2]
        opcode_address[-3] = OPC_STX_ABS;              // STX save_zp  [4]
        opcode_address[-2] = lsb(opcode_address + 6);
        opcode_address[-1] = msb(opcode_address + 6);
        opcode_address[ 0] = OPC_BRK;                  // BRK          [7]
        opcode_address[ 1] = OPC_LDX_ABS;              // LDX save_zp  [4]
        opcode_address[ 2] = lsb(opcode_address + 6);  //
        opcode_address[ 3] = msb(opcode_address + 6);  //
        opcode_address[ 4] = OPC_TXS;                  // TXS          [2]
        opcode_address[ 5] = OPC_RTS;                  // RTS          [-] Return address for the BRK.

        m_test_overhead_cycles = 2 + 4 + TARGET_SPECIFIC_IRQ_OVERHEAD + 4 + 2;
        m_instruction_cycles   = 7;

        oldvec = set_irq_vector_address(opcode_address + 1);

        proceed = execute_single_opcode_test(opcode_address - 4, DEFAULT_RUN_FLAGS);

        set_irq_vector_address(oldvec);

        if (!proceed)
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_rti_instruction(const char * opcode_description)
{
    uint8_t * opcode_address;

    prepare_opcode_tests(opcode_description, Par1_OpcodeOffset);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[-7] = OPC_LDA_IMM;              // LDA #>rts_address     [2]
        opcode_address[-6] = msb(opcode_address + 1);  //
        opcode_address[-5] = OPC_PHA;                  // PHA                   [3]
        opcode_address[-4] = OPC_LDA_IMM;              // LDA #<rts_address     [2]
        opcode_address[-3] = lsb(opcode_address + 1);  //
        opcode_address[-2] = OPC_PHA;                  // PHA                   [3]
        opcode_address[-1] = OPC_PHP;                  // PHP                   [3]
        opcode_address[ 0] = OPC_RTI;                  // RTI                   [6]
        opcode_address[ 1] = OPC_RTS;                  // RTS                   [-]

        m_test_overhead_cycles = 2 + 3 + 2 + 3 + 3;
        m_instruction_cycles   = 6;

        if (!execute_single_opcode_test(opcode_address - 7, DEFAULT_RUN_FLAGS))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}
