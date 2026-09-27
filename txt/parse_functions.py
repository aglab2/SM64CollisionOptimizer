#!/usr/bin/env python3
"""Parse ram_addresses.txt and write functions.json."""

import json


ROM_REGIONS = [
    (0x80378800, 0x000f5580),
    (0x80246000, 0x00001000),
]


def ram_to_rom(ram_addr: int) -> str | None:
    for ram_start, rom_start in ROM_REGIONS:
        if ram_addr >= ram_start:
            return f"0x{rom_start + (ram_addr - ram_start):012X}"
    return None


def main():
    functions = []
    with open("ram_addresses.txt") as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 2:
                functions.append((parts[1], int(parts[0], 16)))

    functions.sort(key=lambda x: x[1])

    result = []
    for i, (name, addr) in enumerate(functions):
        entry = {
            "name": name,
            "ram_addr": f"0x{addr:X}",
            "rom_addr": ram_to_rom(addr) or "unmapped",
        }
        if i < len(functions) - 1:
            delta = functions[i + 1][1] - addr
            if delta > 0:
                entry["length"] = delta
            else:
                entry["length"] = None
        else:
            entry["length"] = None
        result.append(entry)

    with open("functions.json", "w") as f:
        json.dump(result, f, indent=2)


if __name__ == "__main__":
    main()
