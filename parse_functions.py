#!/usr/bin/env python3
"""Parse ram_addresses.txt and compute the length of each function.

Assumes all functions are contiguous — the end of one function is the start
of the next. The last function's length cannot be determined this way.
"""

import sys
from dataclasses import dataclass


@dataclass
class Function:
    name: str
    address: int


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


def compute_lengths(functions: list[Function]) -> list[tuple[str, int, int | str]]:
    """Compute (name, start_addr, length) for each function.

    The last function gets None for length since we don't know where it ends.
    Gap entries are inserted where contiguity breaks (negative or zero delta).
    """
    results = []
    for i, func in enumerate(functions):
        if i < len(functions) - 1:
            delta = functions[i + 1].address - func.address
            if delta <= 0:
                # Contiguity break — insert a gap marker before the next function
                results.append((func.name, func.address, None))
                results.append(
                    ("*** GAP ***", functions[i + 1].address, f"gap of {-delta} bytes")
                )
            else:
                results.append((func.name, func.address, delta))
        else:
            results.append((func.name, func.address, None))
    return results


def main():
    filepath = sys.argv[1] if len(sys.argv) > 1 else "ram_addresses.txt"
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

    print(f"{'Function':<{name_width}}  {'Address':>{addr_width}}  {'Length (bytes)':>15}  {'Length (hex)':>12}")
    print("-" * (name_width + addr_width + 15 + 12 + 8))

    total_bytes = 0
    for name, addr, length in results:
        if isinstance(length, int):
            total_bytes += length
            print(f"{name:<{name_width}}  {addr:>#{addr_width}X}  {length:>15}  {length:>{12}X}")
        elif length is None:
            print(f"{name:<{name_width}}  {addr:>#{addr_width}X}  {'(unknown)':>15}  {'':>12}")
        else:
            # Gap marker
            print(f"{name:<{name_width}}  {addr:>#{addr_width}X}  {length:>15}")

    print("-" * (name_width + addr_width + 15 + 12 + 8))
    if total_bytes > 0:
        print(f"Total contiguous span: {total_bytes:,} bytes ({total_bytes / 1024:.1f} KB)")
    else:
        print("Note: addresses are not in strict ascending order — multiple blocks detected.")
    print(f"Number of functions:   {len(functions)}")


if __name__ == "__main__":
    main()
