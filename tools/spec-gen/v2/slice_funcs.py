#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
slice_funcs.py — extract each screen-drawing function body from the body FW.

This is the "manager/routing" step: it carves `uint8_t disp_XXX(...) { ... }`
out of ../IM-110/Core/Src/Display.c so each function's REAL source can be handed
to Devstral. It does NOT interpret behavior — Devstral reads the code and writes
the spec. Output: v2/code/<func>.c  +  v2/func_index.json.

Run:  python3 tools/spec-gen/v2/slice_funcs.py
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[2]
SRC = REPO.parent / "IM-110" / "Core" / "Src" / "Display.c"
CODE = ROOT / "code"

DEF_RE = re.compile(r"^uint8_t\s+(disp_[A-Za-z0-9_]+)\s*\(")


def slice_functions(text: str) -> list[dict]:
    lines = text.splitlines()
    funcs = []
    i = 0
    n = len(lines)
    while i < n:
        m = DEF_RE.match(lines[i])
        if not m:
            i += 1
            continue
        name = m.group(1)
        start = i
        # find opening brace (same line or following lines)
        depth = 0
        seen_open = False
        j = i
        while j < n:
            depth += lines[j].count("{") - lines[j].count("}")
            if "{" in lines[j]:
                seen_open = True
            if seen_open and depth <= 0:
                break
            j += 1
        end = j
        body = "\n".join(lines[start:end + 1])
        funcs.append({"name": name, "start": start + 1, "end": end + 1,
                      "lines": end - start + 1, "src": body})
        i = end + 1
    return funcs


def main() -> int:
    CODE.mkdir(parents=True, exist_ok=True)
    text = SRC.read_text(encoding="utf-8", errors="ignore")
    funcs = slice_functions(text)
    index = []
    for f in funcs:
        (CODE / f"{f['name']}.c").write_text(f["src"], encoding="utf-8")
        index.append({k: f[k] for k in ("name", "start", "end", "lines")})
    (ROOT / "func_index.json").write_text(
        json.dumps(index, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"sliced {len(funcs)} functions from {SRC.name} -> {CODE}")
    print("longest:", sorted(index, key=lambda x: -x["lines"])[:5])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
