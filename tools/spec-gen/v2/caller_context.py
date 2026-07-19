#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
caller_context.py — extract, per screen function, the caller state-machine blocks.

② transitions/switch handling live NOT in Display.c (drawing) but in the callers
Normal.c / Setting.c / Adjust.c, inside `switch(operation_mode){ case X: ... }`
blocks that both call disp_* and set the next operation_mode on MEM/DISP switch
input. This is the "manager/routing" step: for each disp_* function it collects
the caller case-blocks that reference it, so Devstral can read the real
transition logic (it does not interpret — Devstral does).

Outputs: v2/caller/<func>.c  +  v2/caller_index.json
Also copies AllDef.h excerpt (the operation_mode state dictionary) to v2/AllDef_excerpt.h

Run:  python3 tools/spec-gen/v2/caller_context.py
"""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
FW = ROOT.parents[2].parent / "IM-110" / "Core"
SRC = FW / "Src"
CALLERS = ["Normal.c", "Setting.c", "Adjust.c", "mainSub.c"]
CALLER_DIR = ROOT / "caller"

LABEL_RE = re.compile(r"^\s*(case\s+[A-Za-z0-9_]+|default)\s*:")
DISP_RE = re.compile(r"\b(disp_[A-Za-z0-9_]+)\s*\(")


def split_case_blocks(text: str, fname: str) -> list[dict]:
    """Split a caller file into segments starting at each case/default label."""
    lines = text.splitlines()
    blocks = []
    cur_label = None
    cur_start = None
    buf = []
    for i, ln in enumerate(lines):
        m = LABEL_RE.match(ln)
        if m:
            if cur_label is not None:
                blocks.append({"file": fname, "label": cur_label,
                               "start": cur_start + 1, "src": "\n".join(buf)})
            cur_label = m.group(1).strip()
            cur_start = i
            buf = [ln]
        elif cur_label is not None:
            buf.append(ln)
    if cur_label is not None:
        blocks.append({"file": fname, "label": cur_label,
                       "start": cur_start + 1, "src": "\n".join(buf)})
    return blocks


def main() -> int:
    CALLER_DIR.mkdir(parents=True, exist_ok=True)
    all_blocks = []
    for fn in CALLERS:
        p = SRC / fn
        if not p.exists():
            print("  (missing)", fn)
            continue
        all_blocks += split_case_blocks(p.read_text(encoding="utf-8", errors="ignore"), fn)

    # map disp_func -> list of blocks referencing it
    func_blocks: dict[str, list] = {}
    for b in all_blocks:
        for fnname in set(DISP_RE.findall(b["src"])):
            func_blocks.setdefault(fnname, []).append(b)

    index = {}
    for fnname, blocks in func_blocks.items():
        # cap block size so a giant case doesn't dominate the prompt
        parts = []
        for b in blocks:
            src = b["src"]
            if len(src.splitlines()) > 120:
                src = "\n".join(src.splitlines()[:120]) + "\n// ...(case block truncated)"
            parts.append(f"// ===== {b['file']} : {b['label']} (L{b['start']}) =====\n{src}")
        (CALLER_DIR / f"{fnname}.c").write_text("\n\n".join(parts), encoding="utf-8")
        index[fnname] = [{"file": b["file"], "label": b["label"], "start": b["start"]}
                         for b in blocks]

    (ROOT / "caller_index.json").write_text(
        json.dumps(index, ensure_ascii=False, indent=2), encoding="utf-8")

    # AllDef.h state dictionary (operation_mode families + sub-states)
    alldef = FW / "Inc" / "AllDef.h"
    if alldef.exists():
        (ROOT / "AllDef_excerpt.h").write_text(
            alldef.read_text(encoding="utf-8", errors="ignore"), encoding="utf-8")

    covered = len(index)
    total_funcs = len(json.loads((ROOT / "func_index.json").read_text(encoding="utf-8")))
    print(f"caller context: {len(all_blocks)} case-blocks scanned, "
          f"{covered}/{total_funcs} disp_* funcs have caller context")
    no_ctx = [f["name"] for f in json.loads((ROOT / "func_index.json").read_text(encoding="utf-8"))
              if f["name"] not in index]
    print(f"no caller context ({len(no_ctx)}):", no_ctx[:15])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
