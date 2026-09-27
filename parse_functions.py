#!/usr/bin/env python3
"""Parse ram_addresses.txt and compute the length of each function.

Assumes all functions are contiguous — the end of one function is the start
of the next. The last function's length cannot be determined this way.
"""

import json
import sys
from dataclasses import dataclass


@dataclass
class Function:
    name: str
    address: int


# RAM → ROM mapping regions: (ram_start, rom_start)
ROM_REGIONS = [
    (0x80246000, 0x00001000),
    (0x80378800, 0x000f5580),
]


def ram_to_rom(ram_addr: int) -> str | None:
    """Convert a RAM address to its ROM equivalent using the known mapping regions."""
    for ram_start, rom_start in ROM_REGIONS:
        if ram_addr >= ram_start:
            offset = ram_addr - ram_start
            return f"0x{rom_start + offset:012X}"
    return None


def parse_addresses(filepath: str) -> list[Function]:
    """Parse the ram_addresses.txt file and return a list of Functions."""
    functions = []
    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) >= 2:
                address = int(parts[0], 16)
                name = parts[1]
                functions.append(Function(name=name, address=address))
    return functions


def get_region(ram_addr: int) -> int | None:
    """Return the region index for a RAM address, or None if unmapped.

    Picks the region with the largest ram_start that is still <= the address,
    so higher regions take priority when ranges overlap.
    """
    best = None
    for idx, (ram_start, _) in enumerate(ROM_REGIONS):
        if ram_addr >= ram_start:
            if best is None or ram_start > ROM_REGIONS[best][0]:
                best = idx
    return best


def compute_lengths(functions: list[Function]) -> list[tuple[str, int, int | str]]:
    """Compute (name, start_addr, length) for each function.

    The last function gets None for length since we don't know where it ends.
    Gap entries are inserted where contiguity breaks or regions change.
    """
    results = []
    for i, func in enumerate(functions):
        if i < len(functions) - 1:
            next_func = functions[i + 1]
            delta = next_func.address - func.address
            cur_region = get_region(func.address)
            next_region = get_region(next_func.address)

            if delta <= 0 or cur_region != next_region:
                # Contiguity break or region boundary — insert a gap marker
                results.append((func.name, func.address, None))
                reason = f"gap of {-delta} bytes"
                if cur_region is not None and next_region is not None and cur_region != next_region:
                    reason = f"region {cur_region}→{next_region}, gap of {-delta} bytes"
                results.append(
                    ("*** GAP ***", next_func.address, reason)
                )
            else:
                results.append((func.name, func.address, delta))
        else:
            results.append((func.name, func.address, None))
    return results


def build_json_data(functions: list[Function]) -> list[dict]:
    """Build a JSON-serializable list of function dicts."""
    results = compute_lengths(functions)
    output = []
    for name, addr, length in results:
        entry: dict = {
            "name": name,
            "ram_addr": f"0x{addr:X}",
            "rom_addr": ram_to_rom(addr) or "unmapped",
        }
        if isinstance(length, int):
            entry["length_bytes"] = length
            entry["length_hex"] = f"0x{length:X}"
        elif length is None:
            entry["length_bytes"] = None  # last function or region boundary
            entry["length_hex"] = None
        else:
            # Gap marker — skip these entirely from JSON
            continue
        output.append(entry)
    return output


def main():
    args = sys.argv[1:]
    filepath = args[0] if args else "ram_addresses.txt"
    save_path = None
    i = 1
    while i < len(args):
        if args[i] == "--save" and i + 1 < len(args):
            save_path = args[i + 1]
            i += 2
        else:
            i += 1

    functions = parse_addresses(filepath)

    if not functions:
        print("No functions found.")
        return

    # Sort by address so contiguous blocks are in order
    functions.sort(key=lambda f: f.address)

    results = compute_lengths(functions)

    # Determine column widths for alignment
    name_width = max(len(name) for name, _, _ in results)
    addr_width = max(len(f"0x{addr:X}") for _, addr, _ in results)
    rom_width = 16  # fixed width for ROM addresses (e.g. 0x000f5a80)

    print(f"{'Function':<{name_width}}  {'RAM Addr':>{addr_width}}  {'ROM Addr':>{rom_width}}  {'Len (B)':>7}  {'Len (h)':>7}")
    print("-" * (name_width + addr_width + rom_width + 7 + 7 + 12))

    total_bytes = 0
    for name, addr, length in results:
        rom_addr = ram_to_rom(addr)
        rom_str = f"{rom_addr}" if rom_addr else "(unmapped)"
        if isinstance(length, int):
            total_bytes += length
            print(f"{name:<{name_width}}  {addr:>#{addr_width}X}  {rom_str:>{rom_width}}  {length:>7}  {length:>7X}")
        elif length is None:
            print(f"{name:<{name_width}}  {addr:>#{addr_width}X}  {rom_str:>{rom_width}}  {'(end)':>7}")
        else:
            # Gap marker — don't add to total
            print(f"{name:<{name_width}}  {addr:>#{addr_width}X}  {rom_str:>{rom_width}}  {str(length):>19}")

    print("-" * (name_width + addr_width + 15 + 12 + 8))
    if total_bytes > 0:
        print(f"Total contiguous span: {total_bytes:,} bytes ({total_bytes / 1024:.1f} KB)")
    else:
        print("Note: addresses are not in strict ascending order — multiple blocks detected.")
    print(f"Number of functions:   {len(functions)}")

    # Save JSON if requested
    if save_path:
        json_data = build_json_data(functions)
        with open(save_path, "w") as f:
            json.dump(json_data, f, indent=2)
        print(f"\nSaved {len(json_data)} functions to {save_path}")


if __name__ == "__main__":
    main()
