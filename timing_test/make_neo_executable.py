#! /usr/bin/env python3

import struct
import argparse


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("inputfile")
    parser.add_argument("outputfile")

    args = parser.parse_args()

    with open(args.inputfile, "rb") as fi:
        data = fi.read()

    start_address = 0x0200
    load_address = 0x0200
    size = len(data)

    header = struct.pack("<B3sBBHBHHB",
                  3,                # NEO length prefix, part of the 4-byte magical constant.
                  b"NEO",           # Magic constant
                  0, 0,             # Minimum major/minor version required to work.
                  start_address,    # Start address.
                  0,                # Control bits.
                  load_address,     # Load address.
                  size,             # Size of image to load.
                  0                 # Zero-terminated string.
            )

    with open(args.outputfile, "wb") as fo:
        fo.write(header + data)

if __name__ == "__main__":
    main()
