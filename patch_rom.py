#!/usr/bin/env python3
"""Patcher script that copies .bin files from c/bin into rom.z64 at offsets defined by txt/functions.json."""

import json
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BIN_DIR = os.path.join(SCRIPT_DIR, "c", "bin")
FUNCTIONS_JSON = os.path.join(SCRIPT_DIR, "txt", "functions.json")
ROM_PATH = os.path.join(SCRIPT_DIR, "rom.z64")

def main():
    with open(FUNCTIONS_JSON, "r") as f:
        functions = json.load(f)

    func_map = {fn["name"]: fn for fn in functions}

    with open(ROM_PATH, "rb") as f:
        rom_data = bytearray(f.read())

    rom_size = len(rom_data)
    print(f"ROM size: {rom_size:,} bytes")

    # Collect all regions that will be written (to detect gaps later)
    regions = []

    hardcoded_zero = {"max_3", "min_3", "lower_cell_index", "upper_cell_index", "add_surface"}

    # Step 1: Zero out function regions that have .bin files or are in the hardcoded list
    for fn in functions:
        name = fn["name"]
        bin_path = os.path.join(BIN_DIR, f"{name}.bin")
        has_bin = os.path.isfile(bin_path)
        is_hardcoded = name in hardcoded_zero

        if not has_bin and not is_hardcoded:
            continue

        rom_addr = int(fn["rom_addr"], 16)
        length = fn["length"]

        if rom_addr + length > rom_size:
            print(f"  [ERROR] {name}: region exceeds ROM size")
            sys.exit(1)

        for i in range(length):
            rom_data[rom_addr + i] = 0x00

        regions.append((rom_addr, rom_addr + length, name))
        print(f"  [ZERO ] {name}: rom [{fn['rom_addr']}, {hex(rom_addr + length)})")

    # Step 2: Write bin data over the zeros
    for fn in functions:
        name = fn["name"]
        rom_addr = int(fn["rom_addr"], 16)
        length = fn["length"]
        bin_path = os.path.join(BIN_DIR, f"{name}.bin")

        if not os.path.isfile(bin_path):
            continue

        with open(bin_path, "rb") as bf:
            bin_data = bf.read()

        for i, byte in enumerate(bin_data):
            rom_data[rom_addr + i] = byte

        print(f"  [PATCH] {name}: rom [{fn['rom_addr']}, {hex(rom_addr + length)}) | bin={len(bin_data)}b")

    with open(ROM_PATH, "wb") as f:
        f.write(rom_data)

    # Step 3: Verify written content matches source .bin files
    print(f"\nVerification:")
    errors = 0
    for fn in functions:
        name = fn["name"]
        rom_addr = int(fn["rom_addr"], 16)
        length = fn["length"]
        bin_path = os.path.join(BIN_DIR, f"{name}.bin")

        if not os.path.isfile(bin_path):
            continue

        with open(bin_path, "rb") as bf:
            bin_data = bf.read()

        rom_region = bytes(rom_data[rom_addr:rom_addr + len(bin_data)])
        if rom_region == bin_data:
            print(f"  [OK]   {name}: {len(bin_data)} bytes match")
        else:
            print(f"  [FAIL] {name}: mismatch after write!")
            # Find first differing offset
            for i, (a, b) in enumerate(zip(rom_region, bin_data)):
                if a != b:
                    print(f"         First diff at offset {i}: rom=0x{a:02x} bin=0x{b:02x}")
                    break
            errors += 1

    if errors == 0:
        print(f"\nAll checks passed.")
    else:
        print(f"\n{errors} verification error(s)!")
        sys.exit(1)

    print(f"\nWrote patched ROM to {ROM_PATH}")


if __name__ == "__main__":
    main()
