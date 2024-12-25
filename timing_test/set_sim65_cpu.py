#! /usr/bin/env python3

import argparse
from typing import Optional


cpu_int_to_description = {
    0: "6502, undefined opcodes are illegal",
    1: "65C02",
    2: "6502, unofficial opcodes are supported"
}


cpu_argument_to_type = {
    "6502" : 0,
    "65c02": 1,
    "65C02": 1,
    "6502X": 2,
    "6502x": 2
}

def process_sim65_executable_file(filename: str, cpu_type: Optional[int]) -> None:

    with open(filename, "rb") as fi:
        data = fi.read()

    ok = data.startswith(b"sim65") and len(data) >= 12
    if not ok:
        print(f"{filename!r} -- not a sim65 executable (ignored).")
        return

    if cpu_type is None:
        print(f"{filename!r} -- current CPU type is {data[6]} ({cpu_int_to_description.get(data[6], 'Unknown')}).")
        return

    if data[6] == cpu_type:
        print(f"{filename!r} -- CPU type already {cpu_type}, no change needed.")
        return

    print(f"{filename!r} -- changing type from {data[6]} to {cpu_type}.")

    data = bytearray(data)
    data[6] = cpu_type
    with open(filename, "wb") as fo:
        fo.write(data)

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument("--cpu", choices=cpu_argument_to_type.keys(), help="desired CPU type (omit to display current value only)")
    parser.add_argument("filenames", metavar="filename", nargs="+", help="executables to process")
    args = parser.parse_args()

    cpu_type = cpu_argument_to_type.get(args.cpu)
    for filename in args.filenames:
        process_sim65_executable_file(filename, cpu_type)

if __name__ == "__main__":
    main()
