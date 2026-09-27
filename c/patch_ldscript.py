#!/usr/bin/env python3
import sys
import json
from pathlib import Path


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path_to_ld_script>")
        sys.exit(1)

    script_dir = Path(__file__).resolve().parent.parent

    ld_path = Path(sys.argv[1])
    name = ld_path.stem  # e.g. "sm64-api/ld/symbols.ld" -> "symbols"

    with open(script_dir / "txt/functions.json", "r") as f:
        functions = json.load(f)

    func = next((fn for fn in functions if fn["name"] == name), None)
    if not func:
        print(f"Function '{name}' not found in txt/functions.json")
        sys.exit(1)

    ram_addr = func["ram_addr"]
    fn_name = func["name"]

    with open(script_dir / "c/ldscript.tmpl", "r") as f:
        content = f.read()

    patched = content.replace("%%LINKER%%", ram_addr).replace("%%START%%", fn_name)

    with open(sys.argv[1], "w") as f:
        f.write(patched)


if __name__ == "__main__":
    main()
