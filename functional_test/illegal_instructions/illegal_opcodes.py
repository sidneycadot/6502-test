#! /usr/bin/env python3

from typing import NamedTuple

"""
0x93 -- SHA (zp),y
0x9b
0x9c
0x9e
0x9f
"""

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
    mem = dict(state_description["ram"])
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
        mem
    )


def test_93():
    """SHA (zp),y"""
    filename = "65x02/6502/v1/93.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:

        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        PC = initial.PC
        RegA = initial.RegA
        RegX = initial.RegX
        RegY = initial.RegY
        RegS = initial.RegS
        FlagN = initial.FlagN
        FlagV = initial.FlagV
        FlagD = initial.FlagD
        FlagI = initial.FlagI
        FlagZ = initial.FlagZ
        FlagC = initial.FlagC
        Mem = initial.Mem.copy()

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

        assert PC == final.PC
        assert RegA == final.RegA
        assert RegX == final.RegX
        assert RegY == final.RegY
        assert RegS == final.RegS

        assert FlagN == final.FlagN
        assert FlagV == final.FlagV
        assert FlagD == final.FlagD
        assert FlagI == final.FlagI
        assert FlagZ == final.FlagZ
        assert FlagC == final.FlagC
        assert Mem == final.Mem


def test_9b():
    """TAS abs,y"""
    filename = "65x02/6502/v1/9b.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:

        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        # --------------------

        PC = initial.PC
        RegA = initial.RegA
        RegX = initial.RegX
        RegY = initial.RegY
        RegS = initial.RegS
        FlagN = initial.FlagN
        FlagV = initial.FlagV
        FlagD = initial.FlagD
        FlagI = initial.FlagI
        FlagZ = initial.FlagZ
        FlagC = initial.FlagC
        Mem = initial.Mem.copy()

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

        assert PC == final.PC
        assert RegA == final.RegA
        assert RegX == final.RegX
        assert RegY == final.RegY
        assert RegS == final.RegS

        assert FlagN == final.FlagN
        assert FlagV == final.FlagV
        assert FlagD == final.FlagD
        assert FlagI == final.FlagI
        assert FlagZ == final.FlagZ
        assert FlagC == final.FlagC
        assert Mem == final.Mem

def test_9c():
    """This works now."""
    filename = "65x02/6502/v1/9c.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:
        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        assert (initial.PC + 3) % 65536 == final.PC
        assert initial.RegA == final.RegA
        assert initial.RegX == final.RegX
        assert initial.RegY == final.RegY
        assert initial.RegS == final.RegS
        assert initial.FlagN == final.FlagN
        assert initial.FlagV == final.FlagV
        assert initial.FlagD == final.FlagD
        assert initial.FlagI == final.FlagI
        assert initial.FlagZ == final.FlagZ
        assert initial.FlagC == final.FlagC

        assert len(cycles) == 5

        opcode = initial.Mem[(initial.PC + 0) % 65536]
        baselo = initial.Mem[(initial.PC + 1) % 65536]
        basehi = initial.Mem[(initial.PC + 2) % 65536]

        pagecross = (baselo + initial.RegX) > 255
        basehi_inc = (basehi + 1) % 256

        read_address = (baselo + initial.RegX) % 256 + 256 * basehi
        read_value = initial.Mem[read_address]

        write_value = initial.RegY & basehi_inc

        write_address_lo = (baselo + initial.RegX) % 256
        if pagecross:
            write_address_hi = write_value
        else:
            write_address_hi = basehi

        write_address = write_address_lo + write_address_hi * 256

        mycycle0 = [(initial.PC + 0) % 65536, opcode, 'read']
        mycycle1 = [(initial.PC + 1) % 65536, baselo, 'read']
        mycycle2 = [(initial.PC + 2) % 65536, basehi, 'read']
        mycycle3 = [read_address, read_value, 'read']
        mycycle4 = [write_address, write_value, 'write']

        mycycles = [mycycle0, mycycle1, mycycle2, mycycle3, mycycle4]

        assert cycles == mycycles

        #assert write_address == cycles[4][0]

        #print(f"basehi_inc        {basehi_inc:08b}")
        #print()
        #print(f"basehi            {basehi:08b}")
        #print(f"read_value        {read_value:08b}")
        #print(f"A                 {initial.RegA:08b}")
        #print(f"X                 {initial.RegX:08b}")
        #print("----------------  --------")
        #print(f"write_address_hi  {write_address_hi:08b}")

        #print()
        #print()
        #print()

def test_9e():
    """This works now."""
    filename = "65x02/6502/v1/9e.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:
        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        assert len(cycles) == 5

        opcode = initial.Mem[(initial.PC + 0) % 65536]
        baselo = initial.Mem[(initial.PC + 1) % 65536]
        basehi = initial.Mem[(initial.PC + 2) % 65536]

        basehi_inc = (basehi + 1) % 256

        read_address = (baselo + initial.RegY) % 256 + 256 * basehi
        read_value = initial.Mem[read_address]

        write_value = initial.RegX & basehi_inc

        write_address_lo = (baselo + initial.RegY) % 256

        pagecross = (baselo + initial.RegY) > 255
        if pagecross:
            write_address_hi = write_value
        else:
            write_address_hi = basehi

        write_address = write_address_lo + write_address_hi * 256

        mycycle0 = [(initial.PC + 0) % 65536, opcode, 'read']
        mycycle1 = [(initial.PC + 1) % 65536, baselo, 'read']
        mycycle2 = [(initial.PC + 2) % 65536, basehi, 'read']
        mycycle3 = [read_address, read_value, 'read']
        mycycle4 = [write_address, write_value, 'write']

        mycycles = [mycycle0, mycycle1, mycycle2, mycycle3, mycycle4]

        assert cycles == mycycles

def test_9f():
    """This works now."""
    filename = "65x02/6502/v1/9f.json"
    with open(filename, "rb") as fi:
        testcases = json.load(fi)

    for testcase in testcases:
        name = testcase["name"]
        initial = make_machine_state(testcase["initial"])
        final = make_machine_state(testcase["final"])
        cycles = testcase["cycles"]

        assert (initial.PC + 3) % 65536 == final.PC
        assert initial.RegA == final.RegA
        assert initial.RegX == final.RegX
        assert initial.RegY == final.RegY
        assert initial.RegS == final.RegS
        assert initial.FlagN == final.FlagN
        assert initial.FlagV == final.FlagV
        assert initial.FlagD == final.FlagD
        assert initial.FlagI == final.FlagI
        assert initial.FlagZ == final.FlagZ
        assert initial.FlagC == final.FlagC

        assert len(cycles) == 5
        #print(cycles)

        opcode = initial.Mem[(initial.PC + 0) % 65536]
        baselo = initial.Mem[(initial.PC + 1) % 65536]
        basehi = initial.Mem[(initial.PC + 2) % 65536]

        pagecross = (baselo + initial.RegY) > 255
        basehi_inc = (basehi + 1) % 256

        read_address = (baselo + initial.RegY) % 256 + 256 * basehi
        read_value = initial.Mem[read_address]

        write_value = initial.RegA & initial.RegX & basehi_inc

        write_address_lo = (baselo + initial.RegY) % 256
        if pagecross:
            write_address_hi = write_value
        else:
            write_address_hi = basehi

        write_address = write_address_lo + write_address_hi * 256

        mycycle0 = [(initial.PC + 0) % 65536, opcode, 'read']
        mycycle1 = [(initial.PC + 1) % 65536, baselo, 'read']
        mycycle2 = [(initial.PC + 2) % 65536, basehi, 'read']
        mycycle3 = [read_address, read_value, 'read']
        mycycle4 = [write_address, write_value, 'write']

        mycycles = [mycycle0, mycycle1, mycycle2, mycycle3, mycycle4]

        assert cycles == mycycles

        #assert write_address == cycles[4][0]

        #print(f"basehi_inc        {basehi_inc:08b}")
        #print()
        #print(f"basehi            {basehi:08b}")
        #print(f"read_value        {read_value:08b}")
        #print(f"A                 {initial.RegA:08b}")
        #print(f"X                 {initial.RegX:08b}")
        #print("----------------  --------")
        #print(f"write_address_hi  {write_address_hi:08b}")

        #print()
        #print()
        #print()

def main():
    test_93()
    test_9b()
    test_9c()
    test_9e()
    test_9f()


if __name__ == "__main__":
    main()
