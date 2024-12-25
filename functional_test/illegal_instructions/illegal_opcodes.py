#! /usr/bin/env python3

from typing import NamedTuple

import json

class MachineState(NamedTuple):
    PC: int
    RegA: int
    RegX: int
    RegY: int
    RegS: int
    FlagN: bool
    FlagV: bool
    FlagD: bool
    FlagI: bool
    FlagZ: bool
    FlagC: bool
    Mem: dict[int, int]


def make_machine_state(state_description):
    return MachineState(
        state_description["pc"],
        state_description["a"],
        state_description["x"],
        state_description["y"],
        state_description["s"],
        (state_description["p"] & 0x80) != 0,
        (state_description["p"] & 0x40) != 0,
        (state_description["p"] & 0x08) != 0,
        (state_description["p"] & 0x04) != 0,
        (state_description["p"] & 0x02) != 0,
        (state_description["p"] & 0x01) != 0,
        dict(state_description["ram"])
    )


def test_93():
    """Replicate the behavior of opcode 0x93: SHA (zp),y"""

    filename = "65x02/6502/v1/93.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:

        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        # --------------------

        PC    = initial.PC
        RegA  = initial.RegA
        RegX  = initial.RegX
        RegY  = initial.RegY
        RegS  = initial.RegS
        FlagN = initial.FlagN
        FlagV = initial.FlagV
        FlagD = initial.FlagD
        FlagI = initial.FlagI
        FlagZ = initial.FlagZ
        FlagC = initial.FlagC
        Mem   = initial.Mem.copy()

        # --------------------

        opcode = Mem[PC]
        mycycle0 = [PC, opcode, 'read']

        PC = (PC + 1) & 0xffff
        zp_ptr_lo = Mem[PC]
        mycycle1 = [PC, zp_ptr_lo, 'read']

        PC = (PC + 1) & 0xffff
        baselo = Mem[zp_ptr_lo]
        mycycle2 = [zp_ptr_lo, baselo, 'read']

        zp_ptr_hi = (zp_ptr_lo + 1) % 256
        basehi = Mem[zp_ptr_hi]
        mycycle3 = [zp_ptr_hi, basehi, 'read']

        basehi_inc = (basehi + 1) % 256
        read_address = (baselo + RegY) % 256 + 256 * basehi
        read_value = Mem[read_address]
        mycycle4 = [read_address, read_value, 'read']

        write_value = RegA & RegX & basehi_inc
        write_address_lo = (baselo + RegY) % 256
        pagecross = (baselo + RegY) > 255
        if pagecross:
            write_address_hi = write_value
        else:
            write_address_hi = basehi
        write_address = write_address_lo + write_address_hi * 256
        Mem[write_address] = write_value
        mycycle5 = [write_address, write_value, 'write']

        mycycles = [mycycle0, mycycle1, mycycle2, mycycle3, mycycle4, mycycle5]

        # --------------------

        assert mycycles == cycles

        assert PC    == final.PC
        assert RegA  == final.RegA
        assert RegX  == final.RegX
        assert RegY  == final.RegY
        assert RegS  == final.RegS
        assert FlagN == final.FlagN
        assert FlagV == final.FlagV
        assert FlagD == final.FlagD
        assert FlagI == final.FlagI
        assert FlagZ == final.FlagZ
        assert FlagC == final.FlagC
        assert Mem   == final.Mem


def test_9b():
    """Replicate the behavior of opcode 0x9b: TAS abs,y"""

    filename = "65x02/6502/v1/9b.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:

        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        # --------------------

        PC    = initial.PC
        RegA  = initial.RegA
        RegX  = initial.RegX
        RegY  = initial.RegY
        RegS  = initial.RegS
        FlagN = initial.FlagN
        FlagV = initial.FlagV
        FlagD = initial.FlagD
        FlagI = initial.FlagI
        FlagZ = initial.FlagZ
        FlagC = initial.FlagC
        Mem   = initial.Mem.copy()

        # --------------------

        opcode = Mem[PC]
        mycycle0 = [PC, opcode, 'read']

        PC = (PC + 1) & 0xffff
        baselo = Mem[PC]
        mycycle1 = [PC, baselo, 'read']

        PC = (PC + 1) & 0xffff
        basehi = Mem[PC]
        mycycle2 = [PC, basehi, 'read']

        PC = (PC + 1) & 0xffff
        basehi_inc = (basehi + 1) % 256
        read_address = (baselo + RegY) % 256 + 256 * basehi
        read_value = Mem[read_address]
        mycycle3 = [read_address, read_value, 'read']

        RegS = RegA & RegX
        write_value = RegS & basehi_inc
        write_address_lo = (baselo + RegY) % 256
        pagecross = (baselo + RegY) > 255
        if pagecross:
            write_address_hi = write_value
        else:
            write_address_hi = basehi
        write_address = write_address_lo + write_address_hi * 256
        Mem[write_address] = write_value
        mycycle4 = [write_address, write_value, 'write']

        mycycles = [mycycle0, mycycle1, mycycle2, mycycle3, mycycle4]

        # --------------------

        assert mycycles == cycles

        assert PC    == final.PC
        assert RegA  == final.RegA
        assert RegX  == final.RegX
        assert RegY  == final.RegY
        assert RegS  == final.RegS
        assert FlagN == final.FlagN
        assert FlagV == final.FlagV
        assert FlagD == final.FlagD
        assert FlagI == final.FlagI
        assert FlagZ == final.FlagZ
        assert FlagC == final.FlagC
        assert Mem   == final.Mem

def test_9c():
    """Replicate the behavior of opcode 0x9b: SHY abs,x"""

    filename = "65x02/6502/v1/9c.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:

        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        # --------------------

        PC    = initial.PC
        RegA  = initial.RegA
        RegX  = initial.RegX
        RegY  = initial.RegY
        RegS  = initial.RegS
        FlagN = initial.FlagN
        FlagV = initial.FlagV
        FlagD = initial.FlagD
        FlagI = initial.FlagI
        FlagZ = initial.FlagZ
        FlagC = initial.FlagC
        Mem   = initial.Mem.copy()

        # --------------------

        opcode = Mem[PC]
        mycycle0 = [PC, opcode, 'read']

        PC = (PC + 1) & 0xffff
        baselo = Mem[PC]
        mycycle1 = [PC, baselo, 'read']

        PC = (PC + 1) & 0xffff
        basehi = Mem[PC]
        mycycle2 = [PC, basehi, 'read']

        PC = (PC + 1) & 0xffff
        basehi_inc = (basehi + 1) % 256
        read_address = (baselo + RegX) % 256 + 256 * basehi
        read_value = Mem[read_address]
        mycycle3 = [read_address, read_value, 'read']

        write_value = RegY & basehi_inc
        write_address_lo = (baselo + RegX) % 256
        pagecross = (baselo + RegX) > 255
        if pagecross:
            write_address_hi = write_value
        else:
            write_address_hi = basehi
        write_address = write_address_lo + write_address_hi * 256
        Mem[write_address] = write_value
        mycycle4 = [write_address, write_value, 'write']

        mycycles = [mycycle0, mycycle1, mycycle2, mycycle3, mycycle4]

        # --------------------

        assert mycycles == cycles

        assert PC    == final.PC
        assert RegA  == final.RegA
        assert RegX  == final.RegX
        assert RegY  == final.RegY
        assert RegS  == final.RegS
        assert FlagN == final.FlagN
        assert FlagV == final.FlagV
        assert FlagD == final.FlagD
        assert FlagI == final.FlagI
        assert FlagZ == final.FlagZ
        assert FlagC == final.FlagC
        assert Mem   == final.Mem

def test_9e():
    """Replicate the behavior of opcode 0x9e: SHX abs,y"""

    filename = "65x02/6502/v1/9e.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:

        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        # --------------------

        PC    = initial.PC
        RegA  = initial.RegA
        RegX  = initial.RegX
        RegY  = initial.RegY
        RegS  = initial.RegS
        FlagN = initial.FlagN
        FlagV = initial.FlagV
        FlagD = initial.FlagD
        FlagI = initial.FlagI
        FlagZ = initial.FlagZ
        FlagC = initial.FlagC
        Mem   = initial.Mem.copy()

        # --------------------

        opcode = Mem[PC]
        mycycle0 = [PC, opcode, 'read']

        PC = (PC + 1) & 0xffff
        baselo = Mem[PC]
        mycycle1 = [PC, baselo, 'read']

        PC = (PC + 1) & 0xffff
        basehi = Mem[PC]
        mycycle2 = [PC, basehi, 'read']

        PC = (PC + 1) & 0xffff
        basehi_inc = (basehi + 1) % 256
        read_address = (baselo + RegY) % 256 + 256 * basehi
        read_value = Mem[read_address]
        mycycle3 = [read_address, read_value, 'read']

        write_value = RegX & basehi_inc
        write_address_lo = (baselo + RegY) % 256
        pagecross = (baselo + RegY) > 255
        if pagecross:
            write_address_hi = write_value
        else:
            write_address_hi = basehi
        write_address = write_address_lo + write_address_hi * 256
        Mem[write_address] = write_value
        mycycle4 = [write_address, write_value, 'write']

        mycycles = [mycycle0, mycycle1, mycycle2, mycycle3, mycycle4]

        # --------------------

        assert mycycles == cycles

        assert PC    == final.PC
        assert RegA  == final.RegA
        assert RegX  == final.RegX
        assert RegY  == final.RegY
        assert RegS  == final.RegS
        assert FlagN == final.FlagN
        assert FlagV == final.FlagV
        assert FlagD == final.FlagD
        assert FlagI == final.FlagI
        assert FlagZ == final.FlagZ
        assert FlagC == final.FlagC
        assert Mem   == final.Mem


def test_9f():
    """Replicate the behavior of opcode 0x9f: SHA abs,y"""

    filename = "65x02/6502/v1/9f.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:

        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        # --------------------

        PC    = initial.PC
        RegA  = initial.RegA
        RegX  = initial.RegX
        RegY  = initial.RegY
        RegS  = initial.RegS
        FlagN = initial.FlagN
        FlagV = initial.FlagV
        FlagD = initial.FlagD
        FlagI = initial.FlagI
        FlagZ = initial.FlagZ
        FlagC = initial.FlagC
        Mem   = initial.Mem.copy()

        # --------------------

        opcode = Mem[PC]
        mycycle0 = [PC, opcode, 'read']

        PC = (PC + 1) & 0xffff
        baselo = Mem[PC]
        mycycle1 = [PC, baselo, 'read']

        PC = (PC + 1) & 0xffff
        basehi = Mem[PC]
        mycycle2 = [PC, basehi, 'read']

        PC = (PC + 1) & 0xffff
        basehi_inc = (basehi + 1) % 256
        read_address = (baselo + RegY) % 256 + 256 * basehi
        read_value = Mem[read_address]
        mycycle3 = [read_address, read_value, 'read']

        write_value = RegA & RegX & basehi_inc
        write_address_lo = (baselo + RegY) % 256
        pagecross = (baselo + RegY) > 255
        if pagecross:
            write_address_hi = write_value
        else:
            write_address_hi = basehi
        write_address = write_address_lo + write_address_hi * 256
        Mem[write_address] = write_value
        mycycle4 = [write_address, write_value, 'write']

        mycycles = [mycycle0, mycycle1, mycycle2, mycycle3, mycycle4]

        # --------------------

        assert mycycles == cycles

        assert PC    == final.PC
        assert RegA  == final.RegA
        assert RegX  == final.RegX
        assert RegY  == final.RegY
        assert RegS  == final.RegS
        assert FlagN == final.FlagN
        assert FlagV == final.FlagV
        assert FlagD == final.FlagD
        assert FlagI == final.FlagI
        assert FlagZ == final.FlagZ
        assert FlagC == final.FlagC
        assert Mem   == final.Mem


def main():
    test_93()
    test_9b()
    test_9c()
    test_9e()
    test_9f()


if __name__ == "__main__":
    main()
