#!/usr/bin/env python3
"""Verify that .bin files in c/bin do not overflow their expected lengths from txt/functions.json."""

import json
import os
import re
import sys

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
FUNCTIONS_JSON = os.path.join(BASE_DIR, "txt", "functions.json")
BIN_DIR = os.path.join(BASE_DIR, "c", "bin")
LOCALS_LD = os.path.join(BASE_DIR, "c", "locals.ld")


def parse_locals_ld():
    """Parse locals.ld to get symbol -> relocated address mapping."""
    reloc_map = {}
    if not os.path.exists(LOCALS_LD):
        return reloc_map
    with open(LOCALS_LD) as f:
        for line in f:
            line = line.strip()
            m = re.match(r'(\w+)\s*=\s*(0x[0-9a-fA-F]+);', line)
            if m:
                reloc_map[m.group(1)] = int(m.group(2), 16)
    return reloc_map


def build_contiguous_layout(functions, reloc_map):
    """Build a list of (relocated_addr, func_info) sorted by relocated address."""
    entries = []
    for func in functions:
        name = func["name"]
        orig_addr = int(func["ram_addr"], 16)
        length = func["length"]
        relocated_addr = reloc_map.get(name, orig_addr)
        entries.append({
            "name": name,
            "orig_addr": orig_addr,
            "relocated_addr": relocated_addr,
            "length": length,
        })
    # Sort by relocated address
    entries.sort(key=lambda e: e["orig_addr"])
    return entries


def main():
    with open(FUNCTIONS_JSON) as f:
        functions = json.load(f)

    expected = {func["name"]: func["length"] for func in functions}
    reloc_map = parse_locals_ld()
    layout = build_contiguous_layout(functions, reloc_map)

    # Determine which functions have bins
    has_bin = set()
    for entry in layout:
        bin_path = os.path.join(BIN_DIR, f"{entry['name']}.bin")
        if os.path.exists(bin_path):
            has_bin.add(entry["name"])

    errors = []
    spanning_groups = []  # (parent_name, [(missing_name, missing_len), ...])
    orphan_missing = []   # missing functions not contiguous with any parent
    extra = []
    ok_count = 0

    for entry in layout:
        name = entry["name"]
        exp_len = entry["length"]
        reloc = entry["relocated_addr"]

        if name not in has_bin:
            orphan_missing.append((name, exp_len))
            continue

        # Calculate effective limit: own length + lengths of contiguous following MISSING functions
        # Find this entry's position in the layout
        my_idx = None
        for i, e in enumerate(layout):
            if e["orig_addr"] == reloc:
                my_idx = i
                break

        assert my_idx, f"Failed to find sym for name {name}"

        start_entry = layout[my_idx]
        limit = start_entry["length"]
        span_group = []
        for i in range(my_idx + 1, len(layout)):
            next_entry = layout[i]
            if next_entry["orig_addr"] != start_entry["orig_addr"] + limit:
                print(f"Non contig {start_entry["name"]} -> {next_entry["name"]} because {next_entry["orig_addr"]:x} {start_entry["orig_addr"] + limit:x}")
                break

            if next_entry["name"] in has_bin:
                break  # Has a bin file - stop here

            # Missing function - span into it
            span_group.append((next_entry["name"], next_entry["length"]))
            limit += next_entry["length"]
        if span_group:
            spanning_groups.append((start_entry["name"], span_group))

        actual_len = os.path.getsize(os.path.join(BIN_DIR, f"{name}.bin"))
        if actual_len > limit:
            errors.append((name, actual_len, limit))
        elif actual_len == exp_len:
            ok_count += 1

    if os.path.isdir(BIN_DIR):
        existing_files = {f[:-4] for f in os.listdir(BIN_DIR) if f.endswith(".bin")}
        extra = sorted(existing_files - set(expected.keys()))

    # Report: OVERFLOW first (fatal), then MISSING, EXTRA, OK
    if errors:
        for name, actual, expected_len in sorted(errors, key=lambda x: -x[1] / max(x[2], 1)):
            overflow = actual - expected_len
            pct = (overflow / expected_len * 100) if expected_len else float("inf")
            print(f"OVERFLOW: {name}.bin is {actual} bytes, exceeds limit of {expected_len} by {overflow} ({pct:.1f}%)")
    all_missing = []
    if spanning_groups:
        for parent_name, group in spanning_groups:
            total_len = sum(length for _, length in group)
            func_names = ", ".join(name for name, _ in group)
            print(f"MISSING: {parent_name}.bin spans into missing functions ({func_names}), total expected {total_len} bytes")
            all_missing.extend(group)
    if orphan_missing:
        for name, exp_len in orphan_missing:
            print(f"MISSING: {name}.bin (expected {exp_len} bytes)")
            all_missing.append((name, exp_len))
    if extra:
        for name in extra:
            print(f"EXTRA: {name}.bin not in functions.json")
    if ok_count:
        print(f"OK: {ok_count} file(s) match expected length exactly")

    # MISSING is informational only (not yet built). Only OVERFLOW + EXTRA are fatal.
    if errors or extra:
        parts = []
        if errors:
            parts.append(f"{len(errors)} overflow(s)")
        if extra:
            parts.append(f"{len(extra)} extra")
        print(f"\n{' '.join(parts)}")
        sys.exit(1)
    elif all_missing:
        print(f"\n{len(all_missing)} missing (not fatal)")
        print("\nAll checks passed.")
    else:
        print("\nAll checks passed.")


if __name__ == "__main__":
    main()
